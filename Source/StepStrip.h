#pragma once

#include <JuceHeader.h>
#include "DistortionCurveDisplay.h"

// One step's full set of controls, drawn as a self-contained "card":
// enable + type + curve display + drive/filter knobs.
class StepStrip : public juce::Component
{
public:
    StepStrip (juce::AudioProcessorValueTreeState& apvts, int stepIndex);

    void paint (juce::Graphics&) override;
    void resized() override;

    // Highlights this card when it's the currently playing sequencer step.
    void setActive (bool shouldBeActive);

private:
    using APVTS = juce::AudioProcessorValueTreeState;

    void configureRotary (juce::Slider&);

    juce::ToggleButton enabledButton;
    juce::ComboBox typeBox;
    DistortionCurveDisplay curveDisplay;

    juce::Label driveLabel;
    juce::Slider driveSlider;

    juce::ComboBox filterTypeBox;

    juce::Label cutoffLabel;
    juce::Slider cutoffSlider;

    juce::Label resonanceLabel;
    juce::Slider resonanceSlider;

    std::unique_ptr<APVTS::ButtonAttachment> enabledAttachment;
    std::unique_ptr<APVTS::ComboBoxAttachment> typeAttachment;
    std::unique_ptr<APVTS::SliderAttachment> driveAttachment;
    std::unique_ptr<APVTS::ComboBoxAttachment> filterTypeAttachment;
    std::unique_ptr<APVTS::SliderAttachment> cutoffAttachment;
    std::unique_ptr<APVTS::SliderAttachment> resonanceAttachment;

    bool active = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepStrip)
};
