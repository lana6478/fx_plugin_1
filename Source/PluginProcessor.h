#pragma once

#include <JuceHeader.h>
#include "Distortion.h"
#include "StepFilterType.h"

// Number of steps in the sequencer. Kept fixed for v1 to keep the parameter
// layout simple; a future version could make this switchable.
constexpr int numSteps = 8;

class StepDistortAudioProcessor : public juce::AudioProcessor
{
public:
    StepDistortAudioProcessor();
    ~StepDistortAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Names for the "Step Rate" choice parameter, and the corresponding number
    // of sequencer steps per host quarter note. Shared here so the processor
    // and editor can't disagree about what each choice means.
    static const juce::StringArray& getStepRateNames();
    static const std::array<double, 6>& getStepsPerQuarterNoteTable();

    juce::AudioProcessorValueTreeState apvts;

    // Updated on the audio thread each sample, read by the editor's timer to
    // highlight the currently playing step. Plain atomic int is enough here.
    std::atomic<int> currentStepIndex { 0 };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    double hostBpm = 120.0;
    double internalPpqPosition = 0.0; // free-running fallback when no host transport is available
    double currentSampleRate = 44100.0;

    juce::LinearSmoothedValue<float> smoothedMix;
    juce::LinearSmoothedValue<float> smoothedOutputGain;

    // A frozen copy of one step's parameters, captured at the moment the
    // sequencer switches steps. Frozen so that the "outgoing" side of a
    // crossfade keeps sounding consistent even while the live parameter
    // values underneath it keep changing.
    struct StepSnapshot
    {
        bool enabled = true;
        DistortionType distortionType = DistortionType::clean;
        float drive = 0.0f;
        StepFilterType filterType = StepFilterType::off;
        float filterCutoff = 1000.0f;
        float filterResonance = 0.707f;
    };

    // Two filter voices so one can keep ringing out the previous step's
    // settings while the other takes over the new step, crossfaded between
    // over a duration set by the new step's "smoothness" parameter. Each
    // filter handles all channels internally (StateVariableTPTFilter is
    // multichannel via prepare()/processSample(channel, x)).
    juce::dsp::StateVariableTPTFilter<float> voiceFilterA, voiceFilterB;

    int previousStepIndexProcessed = -1;
    bool voiceAIsActive = true;
    StepSnapshot outgoingSnapshot, incomingSnapshot;
    int crossfadeSamplesRemaining = 0;
    int crossfadeTotalSamples = 1;

    float processStepVoice (juce::dsp::StateVariableTPTFilter<float>& filter,
                             const StepSnapshot& snapshot, float dry, int channel) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepDistortAudioProcessor)
};
