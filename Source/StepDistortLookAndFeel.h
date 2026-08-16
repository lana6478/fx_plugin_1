#pragma once

#include <JuceHeader.h>

// Shared warm, industrial "burning metal" colour palette and typography, used
// by the LookAndFeel and by the custom-painted components (step cards, curve
// display) so everything stays visually consistent.
namespace StepDistortColours
{
    const juce::Colour background   { 0xff0f0908 }; // near-black, warm undertone
    const juce::Colour panel        { 0xff1d1310 }; // dark ember panel
    const juce::Colour panelOutline { 0xff4a2416 }; // dull rust outline
    const juce::Colour cardActive   { 0xff2c150e }; // glowing-ember card background
    const juce::Colour text         { 0xfff5e6d3 }; // warm cream
    const juce::Colour textDim      { 0xffb08867 }; // muted warm tan
    const juce::Colour accent       { 0xffff7a29 }; // bright orange
    const juce::Colour accentAlt    { 0xffff3319 }; // hot red-orange
    const juce::Colour emberHot     { 0xffffd166 }; // yellow-white glow tip
    const juce::Colour emberDark    { 0xff3a0f06 }; // deep smouldering red

    // The industrial/dystopian display face used throughout the UI.
    inline juce::Font industrialFont (float height)
    {
        return juce::Font (juce::FontOptions ("DIN Condensed", height, juce::Font::bold));
    }
}

// Dark, glowing "burning metal" look shared by the whole editor: fire-gradient
// rotary knobs with a soft glow, riveted LED-style toggle buttons, and the
// DIN Condensed Bold industrial typeface applied everywhere.
class StepDistortLookAndFeel : public juce::LookAndFeel_V4
{
public:
    StepDistortLookAndFeel();

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle,
                            float rotaryEndAngle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                        int buttonX, int buttonY, int buttonW, int buttonH,
                        juce::ComboBox&) override;
};
