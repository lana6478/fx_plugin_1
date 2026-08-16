#include "PluginProcessor.h"
#include "PluginEditor.h"

StepDistortAudioProcessorEditor::StepDistortAudioProcessorEditor (StepDistortAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    using APVTS = juce::AudioProcessorValueTreeState;

    // --- top bar: step rate, mix, output gain -----------------------------
    stepRateLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (stepRateLabel);

    stepRateBox.addItemList (StepDistortAudioProcessor::getStepRateNames(), 1);
    addAndMakeVisible (stepRateBox);
    stepRateAttachment = std::make_unique<APVTS::ComboBoxAttachment> (
        processorRef.apvts, "stepRate", stepRateBox);

    mixLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (mixLabel);

    mixSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible (mixSlider);
    mixAttachment = std::make_unique<APVTS::SliderAttachment> (processorRef.apvts, "mix", mixSlider);

    outputGainLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (outputGainLabel);

    outputGainSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    outputGainSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible (outputGainSlider);
    outputGainAttachment = std::make_unique<APVTS::SliderAttachment> (
        processorRef.apvts, "outputGain", outputGainSlider);

    // --- one column of controls per step -----------------------------------
    const auto& typeNames = getDistortionTypeNames();

    for (int s = 0; s < numSteps; ++s)
    {
        auto& c = stepControls[(size_t) s];
        const auto prefix = "step" + juce::String (s);

        c.enabledButton.setButtonText (juce::String (s + 1));
        addAndMakeVisible (c.enabledButton);
        c.enabledAttachment = std::make_unique<APVTS::ButtonAttachment> (
            processorRef.apvts, prefix + "Enabled", c.enabledButton);

        c.typeBox.addItemList (typeNames, 1);
        addAndMakeVisible (c.typeBox);
        c.typeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (
            processorRef.apvts, prefix + "Type", c.typeBox);

        c.driveSlider.setSliderStyle (juce::Slider::LinearVertical);
        c.driveSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (c.driveSlider);
        c.driveAttachment = std::make_unique<APVTS::SliderAttachment> (
            processorRef.apvts, prefix + "Drive", c.driveSlider);
    }

    setSize (820, 420);
    startTimerHz (30);
}

StepDistortAudioProcessorEditor::~StepDistortAudioProcessorEditor()
{
    stopTimer();
}

void StepDistortAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    const int active = processorRef.currentStepIndex.load (std::memory_order_relaxed);
    if (active >= 0 && active < numSteps)
    {
        g.setColour (juce::Colours::darkorange.withAlpha (0.35f));
        g.fillRect (stepControls[(size_t) active].columnBounds);
    }

    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawText ("StepDistort", getLocalBounds().removeFromTop (40), juce::Justification::centred);
}

void StepDistortAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);

    area.removeFromTop (40); // title drawn in paint()

    auto topBar = area.removeFromTop (30);
    stepRateLabel.setBounds (topBar.removeFromLeft (70));
    stepRateBox.setBounds (topBar.removeFromLeft (110));
    topBar.removeFromLeft (20);
    mixLabel.setBounds (topBar.removeFromLeft (40));
    mixSlider.setBounds (topBar.removeFromLeft (150));
    topBar.removeFromLeft (20);
    outputGainLabel.setBounds (topBar.removeFromLeft (90));
    outputGainSlider.setBounds (topBar.removeFromLeft (150));

    area.removeFromTop (16);

    const int colWidth = area.getWidth() / numSteps;

    for (int s = 0; s < numSteps; ++s)
    {
        auto& c = stepControls[(size_t) s];
        auto fullCol = area.removeFromLeft (colWidth);
        c.columnBounds = fullCol;

        auto col = fullCol.reduced (4);
        c.enabledButton.setBounds (col.removeFromTop (24));
        col.removeFromTop (4);
        c.typeBox.setBounds (col.removeFromTop (24));
        col.removeFromTop (6);
        c.driveSlider.setBounds (col);
    }
}

void StepDistortAudioProcessorEditor::timerCallback()
{
    const int current = processorRef.currentStepIndex.load (std::memory_order_relaxed);
    if (current != lastDrawnStep)
    {
        lastDrawnStep = current;
        repaint();
    }
}
