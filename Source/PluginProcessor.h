#pragma once

#include <JuceHeader.h>
#include "Distortion.h"

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepDistortAudioProcessor)
};
