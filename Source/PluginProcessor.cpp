#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::StringArray& StepDistortAudioProcessor::getStepRateNames()
{
    static const juce::StringArray names { "1/4", "1/8", "1/16", "1/8 triplet", "1/16 triplet", "1/32" };
    return names;
}

const std::array<double, 6>& StepDistortAudioProcessor::getStepsPerQuarterNoteTable()
{
    // How many sequencer steps happen per host quarter note, for each choice
    // in getStepRateNames() (same order).
    static const std::array<double, 6> table { 1.0, 2.0, 4.0, 3.0, 6.0, 8.0 };
    return table;
}

StepDistortAudioProcessor::StepDistortAudioProcessor()
    : AudioProcessor (BusesProperties()
                           .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                           .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

StepDistortAudioProcessor::~StepDistortAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout StepDistortAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "stepRate", 1 }, "Step Rate", getStepRateNames(), 2));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix", 1 }, "Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "outputGain", 1 }, "Output Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f), 0.0f));

    const auto& typeNames = getDistortionTypeNames();

    for (int s = 0; s < numSteps; ++s)
    {
        const auto prefix = "step" + juce::String (s);
        const auto stepLabel = "Step " + juce::String (s + 1);

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { prefix + "Enabled", 1 }, stepLabel + " Enabled", true));

        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { prefix + "Type", 1 }, stepLabel + " Type", typeNames, 1));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { prefix + "Drive", 1 }, stepLabel + " Drive",
            juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
    }

    return { params.begin(), params.end() };
}

void StepDistortAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    smoothedMix.reset (sampleRate, 0.02);
    smoothedOutputGain.reset (sampleRate, 0.02);
    internalPpqPosition = 0.0;
}

void StepDistortAudioProcessor::releaseResources()
{
}

bool StepDistortAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

void StepDistortAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto ch = totalNumInputChannels; ch < totalNumOutputChannels; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    // Work out where we are in musical time. If the host gives us a valid,
    // playing transport we follow it; otherwise we fall back to our own
    // free-running clock (e.g. in the Standalone app with no host).
    double ppqAtBlockStart = internalPpqPosition;
    double bpm = hostBpm;

    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            if (auto bpmOpt = position->getBpm())
                bpm = *bpmOpt;

            if (position->getIsPlaying())
            {
                if (auto ppqOpt = position->getPpqPosition())
                    ppqAtBlockStart = *ppqOpt;
            }
        }
    }

    hostBpm = bpm;

    const auto& stepsPerQuarterTable = getStepsPerQuarterNoteTable();
    const int rateIndex = (int) apvts.getRawParameterValue ("stepRate")->load();
    const double stepsPerQuarter = stepsPerQuarterTable[(size_t) juce::jlimit (0, 5, rateIndex)];

    smoothedMix.setTargetValue (apvts.getRawParameterValue ("mix")->load());
    smoothedOutputGain.setTargetValue (
        juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("outputGain")->load()));

    struct StepParams { bool enabled; DistortionType type; float drive; };
    StepParams steps[(size_t) numSteps];

    for (int s = 0; s < numSteps; ++s)
    {
        const auto prefix = "step" + juce::String (s);
        auto& sp = steps[(size_t) s];
        sp.enabled = apvts.getRawParameterValue (prefix + "Enabled")->load() > 0.5f;
        sp.type    = (DistortionType) (int) apvts.getRawParameterValue (prefix + "Type")->load();
        sp.drive   = apvts.getRawParameterValue (prefix + "Drive")->load();
    }

    const double quarterNotesPerSample = (bpm / 60.0) / currentSampleRate;
    double ppq = ppqAtBlockStart;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    for (int n = 0; n < numSamples; ++n)
    {
        ppq += quarterNotesPerSample;

        int stepIdx = (int) std::floor (ppq * stepsPerQuarter) % numSteps;
        if (stepIdx < 0)
            stepIdx += numSteps;

        currentStepIndex.store (stepIdx, std::memory_order_relaxed);

        const auto& sp = steps[(size_t) stepIdx];
        const float mixNow = smoothedMix.getNextValue();
        const float outGainNow = smoothedOutputGain.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            const float dry = data[n];
            const float wet = sp.enabled ? applyDistortion (dry, sp.type, sp.drive) : dry;

            data[n] = (dry * (1.0f - mixNow) + wet * mixNow) * outGainNow;
        }
    }

    internalPpqPosition = ppq;
}

juce::AudioProcessorEditor* StepDistortAudioProcessor::createEditor()
{
    return new StepDistortAudioProcessorEditor (*this);
}

bool StepDistortAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String StepDistortAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StepDistortAudioProcessor::acceptsMidi() const { return false; }
bool StepDistortAudioProcessor::producesMidi() const { return false; }
bool StepDistortAudioProcessor::isMidiEffect() const { return false; }
double StepDistortAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int StepDistortAudioProcessor::getNumPrograms() { return 1; }
int StepDistortAudioProcessor::getCurrentProgram() { return 0; }
void StepDistortAudioProcessor::setCurrentProgram (int) {}
const juce::String StepDistortAudioProcessor::getProgramName (int) { return {}; }
void StepDistortAudioProcessor::changeProgramName (int, const juce::String&) {}

void StepDistortAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void StepDistortAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StepDistortAudioProcessor();
}
