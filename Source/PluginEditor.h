#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class StepDistortAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    explicit StepDistortAudioProcessorEditor (StepDistortAudioProcessor&);
    ~StepDistortAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    StepDistortAudioProcessor& processorRef;

    struct StepControls
    {
        juce::ToggleButton enabledButton;
        juce::ComboBox typeBox;
        juce::Slider driveSlider;
        juce::Rectangle<int> columnBounds; // used to draw the "now playing" highlight

        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enabledAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    };

    std::array<StepControls, numSteps> stepControls;

    juce::Label stepRateLabel { {}, "Step Rate" };
    juce::ComboBox stepRateBox;
    juce::Label mixLabel { {}, "Mix" };
    juce::Slider mixSlider;
    juce::Label outputGainLabel { {}, "Output Gain" };
    juce::Slider outputGainSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> stepRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;

    int lastDrawnStep = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepDistortAudioProcessorEditor)
};
