#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SliderFormatting.h"

StepDistortAudioProcessorEditor::StepDistortAudioProcessorEditor (StepDistortAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    using namespace StepDistortColours;
    using APVTS = juce::AudioProcessorValueTreeState;

    setLookAndFeel (&lookAndFeel);

    const auto headerLabelFont = industrialFont (12.0f).withExtraKerningFactor (0.04f);

    // --- header: title (custom-painted, see paint()) + global controls -----
    stepRateLabel.setText ("STEP RATE", juce::dontSendNotification);
    stepRateLabel.setJustificationType (juce::Justification::centred);
    stepRateLabel.setFont (headerLabelFont);
    addAndMakeVisible (stepRateLabel);

    stepRateBox.addItemList (StepDistortAudioProcessor::getStepRateNames(), 1);
    addAndMakeVisible (stepRateBox);
    stepRateAttachment = std::make_unique<APVTS::ComboBoxAttachment> (
        processorRef.apvts, "stepRate", stepRateBox);

    mixLabel.setText ("MIX", juce::dontSendNotification);
    mixLabel.setJustificationType (juce::Justification::centred);
    mixLabel.setFont (headerLabelFont);
    addAndMakeVisible (mixLabel);

    mixSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    mixSlider.setPopupDisplayEnabled (true, true, this);
    SliderFormatting::configurePercentDisplay (mixSlider);
    addAndMakeVisible (mixSlider);
    mixAttachment = std::make_unique<APVTS::SliderAttachment> (processorRef.apvts, "mix", mixSlider);

    outputGainLabel.setText ("OUTPUT GAIN", juce::dontSendNotification);
    outputGainLabel.setJustificationType (juce::Justification::centred);
    outputGainLabel.setFont (headerLabelFont);
    addAndMakeVisible (outputGainLabel);

    outputGainSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    outputGainSlider.setPopupDisplayEnabled (true, true, this);
    SliderFormatting::configureDbDisplay (outputGainSlider);
    addAndMakeVisible (outputGainSlider);
    outputGainAttachment = std::make_unique<APVTS::SliderAttachment> (
        processorRef.apvts, "outputGain", outputGainSlider);

    smoothnessLabel.setText ("SMOOTHNESS", juce::dontSendNotification);
    smoothnessLabel.setJustificationType (juce::Justification::centred);
    smoothnessLabel.setFont (headerLabelFont);
    addAndMakeVisible (smoothnessLabel);

    smoothnessSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    smoothnessSlider.setPopupDisplayEnabled (true, true, this);
    SliderFormatting::configurePercentDisplay (smoothnessSlider);
    addAndMakeVisible (smoothnessSlider);
    smoothnessAttachment = std::make_unique<APVTS::SliderAttachment> (
        processorRef.apvts, "smoothness", smoothnessSlider);

    // --- one card per step ---------------------------------------------------
    for (int s = 0; s < numSteps; ++s)
    {
        stepStrips[(size_t) s] = std::make_unique<StepStrip> (processorRef.apvts, s);
        addAndMakeVisible (*stepStrips[(size_t) s]);
    }

    setResizable (true, true);
    setResizeLimits (1200, 420, 2600, 900);
    setSize (2040, 560);

    startTimerHz (30);
}

StepDistortAudioProcessorEditor::~StepDistortAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void StepDistortAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace StepDistortColours;

    auto bounds = getLocalBounds().toFloat();

    // Warm furnace-glow vignette: brighter near the header, fading to
    // near-black at the edges.
    juce::ColourGradient vignette (juce::Colour (0xff20100a), bounds.getCentreX(), 20.0f,
                                    background, bounds.getCentreX(), bounds.getBottom(), true);
    g.setGradientFill (vignette);
    g.fillRect (bounds);

    // Glowing molten-metal title logo, italic-slanted to match the step
    // cards, centred in the middle of the header between the knob groups.
    juce::GlyphArrangement glyphs;
    const auto titleFont = industrialFont (46.0f).withExtraKerningFactor (0.04f).italicised();
    glyphs.addLineOfText (titleFont, "CARNAGE", 0.0f, 0.0f);
    juce::Path titlePath;
    glyphs.createPath (titlePath);

    auto textBounds = titlePath.getBounds();
    const float tx = (float) titleBounds.getCentreX() - textBounds.getCentreX();
    const float ty = (float) titleBounds.getBottom() - textBounds.getBottom();
    titlePath.applyTransform (juce::AffineTransform::translation (tx, ty));

    juce::DropShadow titleGlow (accentAlt.withAlpha (0.7f), 22, {});
    titleGlow.drawForPath (g, titlePath);

    auto titlePathBounds = titlePath.getBounds();
    juce::ColourGradient titleFill (emberHot, titlePathBounds.getX(), titlePathBounds.getY(),
                                     accentAlt, titlePathBounds.getX(), titlePathBounds.getBottom(), false);
    g.setGradientFill (titleFill);
    g.fillPath (titlePath);

    // Glowing seam separating the header from the step row.
    auto seam = juce::Rectangle<float> (bounds.getX() + 12.0f, (float) titleBounds.getBottom() + 42.0f,
                                         bounds.getWidth() - 24.0f, 2.0f);
    juce::Path seamPath;
    seamPath.addRectangle (seam);
    juce::DropShadow seamGlow (accentAlt.withAlpha (0.45f), 12, {});
    seamGlow.drawForPath (g, seamPath);

    juce::ColourGradient seamGrad (accentAlt, seam.getX(), 0.0f, emberHot, seam.getRight(), 0.0f, false);
    g.setGradientFill (seamGrad);
    g.fillRect (seam);
}

void StepDistortAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (12);

    auto header = area.removeFromTop (86);

    auto layoutHeaderControl = [] (juce::Rectangle<int>& remaining, int width,
                                    juce::Label& label, juce::Component& control)
    {
        auto slot = remaining.removeFromLeft (width);
        label.setBounds (slot.removeFromTop (18));
        control.setBounds (slot);
    };

    // Left group: Mix, Output Gain.
    auto leftGroup = header.removeFromLeft (100 + 20 + 110);
    layoutHeaderControl (leftGroup, 100, mixLabel, mixSlider);
    leftGroup.removeFromLeft (20);
    layoutHeaderControl (leftGroup, 110, outputGainLabel, outputGainSlider);

    header.removeFromLeft (24);

    // Right group: Step Rate, Smoothness.
    auto rightGroup = header.removeFromRight (130 + 20 + 100);
    auto stepRateSlot = rightGroup.removeFromLeft (130);
    stepRateLabel.setBounds (stepRateSlot.removeFromTop (18));
    stepRateSlot.removeFromTop (2);
    stepRateBox.setBounds (stepRateSlot.removeFromTop (26));
    rightGroup.removeFromLeft (20);
    layoutHeaderControl (rightGroup, 100, smoothnessLabel, smoothnessSlider);

    header.removeFromRight (24);

    // Whatever's left in the middle is the centred title zone (see paint()).
    titleBounds = header;

    area.removeFromTop (10);

    const int spacing = 6;
    const int colWidth = (area.getWidth() - spacing * (numSteps - 1)) / numSteps;

    for (int s = 0; s < numSteps; ++s)
    {
        auto col = area.removeFromLeft (colWidth);
        stepStrips[(size_t) s]->setBounds (col);
        area.removeFromLeft (spacing);
    }
}

void StepDistortAudioProcessorEditor::timerCallback()
{
    const int current = processorRef.currentStepIndex.load (std::memory_order_relaxed);
    if (current != lastDrawnStep)
    {
        if (lastDrawnStep >= 0 && lastDrawnStep < numSteps)
            stepStrips[(size_t) lastDrawnStep]->setActive (false);

        if (current >= 0 && current < numSteps)
            stepStrips[(size_t) current]->setActive (true);

        lastDrawnStep = current;
    }
}
