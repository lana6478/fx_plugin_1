#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "StepStrip.h"
#include "StepDistortLookAndFeel.h"

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
    StepDistortLookAndFeel lookAndFeel;

    std::array<std::unique_ptr<StepStrip>, numSteps> stepStrips;

    // Title is custom-painted (glowing molten-metal text), not a Label, so its
    // bounds are tracked here and set in resized().
    juce::Rectangle<int> titleBounds;

    juce::Label stepRateLabel;
    juce::ComboBox stepRateBox;
    juce::Label mixLabel;
    juce::Slider mixSlider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label outputGainLabel;
    juce::Slider outputGainSlider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label smoothnessLabel;
    juce::Slider smoothnessSlider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> stepRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> smoothnessAttachment;

    int lastDrawnStep = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepDistortAudioProcessorEditor)
};
