#include "StepStrip.h"
#include "Distortion.h"
#include "StepFilterType.h"
#include "StepDistortLookAndFeel.h"
#include "SliderFormatting.h"

void StepStrip::configureRotary (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    // No permanent number readout; show a formatted value bubble only while
    // the knob is actually being dragged.
    s.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    s.setPopupDisplayEnabled (true, true, this);
}

StepStrip::StepStrip (juce::AudioProcessorValueTreeState& apvts, int stepIndex)
    : curveDisplay (apvts, stepIndex)
{
    const auto prefix = "step" + juce::String (stepIndex);
    const auto labelFont = StepDistortColours::industrialFont (13.0f).withExtraKerningFactor (0.03f);

    enabledButton.setButtonText (juce::String (stepIndex + 1));
    addAndMakeVisible (enabledButton);
    enabledAttachment = std::make_unique<APVTS::ButtonAttachment> (apvts, prefix + "Enabled", enabledButton);

    typeBox.addItemList (getDistortionTypeNames(), 1);
    addAndMakeVisible (typeBox);
    typeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (apvts, prefix + "Type", typeBox);

    addAndMakeVisible (curveDisplay);

    driveLabel.setText ("DRIVE", juce::dontSendNotification);
    addAndMakeVisible (driveLabel);
    driveLabel.setJustificationType (juce::Justification::centred);
    driveLabel.setFont (labelFont);
    configureRotary (driveSlider);
    SliderFormatting::configurePercentDisplay (driveSlider);
    addAndMakeVisible (driveSlider);
    driveAttachment = std::make_unique<APVTS::SliderAttachment> (apvts, prefix + "Drive", driveSlider);

    filterTypeBox.addItemList (getStepFilterTypeNames(), 1);
    addAndMakeVisible (filterTypeBox);
    filterTypeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (apvts, prefix + "FilterType", filterTypeBox);

    cutoffLabel.setText ("CUTOFF", juce::dontSendNotification);
    addAndMakeVisible (cutoffLabel);
    cutoffLabel.setJustificationType (juce::Justification::centred);
    cutoffLabel.setFont (labelFont);
    configureRotary (cutoffSlider);
    SliderFormatting::configureHzDisplay (cutoffSlider);
    addAndMakeVisible (cutoffSlider);
    cutoffAttachment = std::make_unique<APVTS::SliderAttachment> (apvts, prefix + "FilterCutoff", cutoffSlider);

    resonanceLabel.setText ("RESO", juce::dontSendNotification);
    addAndMakeVisible (resonanceLabel);
    resonanceLabel.setJustificationType (juce::Justification::centred);
    resonanceLabel.setFont (labelFont);
    configureRotary (resonanceSlider);
    SliderFormatting::configureQDisplay (resonanceSlider);
    addAndMakeVisible (resonanceSlider);
    resonanceAttachment = std::make_unique<APVTS::SliderAttachment> (apvts, prefix + "FilterResonance", resonanceSlider);
}

void StepStrip::setActive (bool shouldBeActive)
{
    if (active != shouldBeActive)
    {
        active = shouldBeActive;
        repaint();
    }
}

void StepStrip::paint (juce::Graphics& g)
{
    using namespace StepDistortColours;

    auto bounds = getLocalBounds().toFloat().reduced (2.0f);

    if (active)
    {
        juce::Path shape;
        shape.addRoundedRectangle (bounds, 6.0f);
        juce::DropShadow glow (accentAlt.withAlpha (0.55f), 18, {});
        glow.drawForPath (g, shape);
    }

    juce::ColourGradient cardGrad (active ? cardActive : panel, bounds.getX(), bounds.getY(),
                                    active ? juce::Colour (0xff180a06) : juce::Colour (0xff17100e),
                                    bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (cardGrad);
    g.fillRoundedRectangle (bounds, 6.0f);

    g.setColour (active ? accentAlt : panelOutline);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, active ? 2.0f : 1.0f);
}

void StepStrip::resized()
{
    auto area = getLocalBounds().reduced (8);

    auto header = area.removeFromTop (24);
    enabledButton.setBounds (header.removeFromLeft (28));
    header.removeFromLeft (4);
    typeBox.setBounds (header);

    area.removeFromTop (6);

    const int curveHeight = juce::jlimit (50, 90, area.getWidth() / 2);
    curveDisplay.setBounds (area.removeFromTop (curveHeight));

    area.removeFromTop (6);
    filterTypeBox.setBounds (area.removeFromTop (22));
    area.removeFromTop (6);

    // Remaining space: a triangle of 3 larger labeled knobs -- Drive front
    // and centre on top, Cutoff/Resonance below it -- rather than a cramped
    // single row, so each knob can be drawn bigger.
    auto layoutKnob = [] (juce::Rectangle<int> cell, juce::Label& label, juce::Slider& slider)
    {
        label.setBounds (cell.removeFromTop (14));
        slider.setBounds (cell);
    };

    auto topRow = area.removeFromTop (juce::roundToInt ((float) area.getHeight() * 0.52f));
    auto driveCell = topRow.withSizeKeepingCentre (juce::jmin (topRow.getWidth(), topRow.getWidth() * 2 / 3),
                                                    topRow.getHeight());
    layoutKnob (driveCell, driveLabel, driveSlider);

    const int half = area.getWidth() / 2;
    layoutKnob (area.removeFromLeft (half), cutoffLabel, cutoffSlider);
    layoutKnob (area, resonanceLabel, resonanceSlider);
}
