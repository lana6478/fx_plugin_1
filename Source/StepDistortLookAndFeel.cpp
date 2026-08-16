#include "StepDistortLookAndFeel.h"

using namespace StepDistortColours;

StepDistortLookAndFeel::StepDistortLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, background);

    setColour (juce::Slider::rotarySliderFillColourId, accent);
    setColour (juce::Slider::rotarySliderOutlineColourId, panelOutline);
    setColour (juce::Slider::thumbColourId, emberHot);
    setColour (juce::Slider::trackColourId, accent);
    setColour (juce::Slider::backgroundColourId, panel);
    setColour (juce::Slider::textBoxTextColourId, text);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    setColour (juce::ComboBox::backgroundColourId, panel);
    setColour (juce::ComboBox::outlineColourId, panelOutline);
    setColour (juce::ComboBox::textColourId, text);
    setColour (juce::ComboBox::arrowColourId, accent);
    setColour (juce::PopupMenu::backgroundColourId, panel);
    setColour (juce::PopupMenu::textColourId, text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, accentAlt.withAlpha (0.4f));

    setColour (juce::ToggleButton::textColourId, text);
    setColour (juce::ToggleButton::tickColourId, accentAlt);
    setColour (juce::ToggleButton::tickDisabledColourId, textDim);

    setColour (juce::Label::textColourId, text);

    setColour (juce::TextButton::buttonColourId, panel);
    setColour (juce::TextButton::textColourOffId, textDim);
    setColour (juce::TextButton::textColourOnId, background);
    setColour (juce::TextButton::buttonOnColourId, accentAlt);
}

juce::Typeface::Ptr StepDistortLookAndFeel::getTypefaceForFont (const juce::Font& font)
{
    auto industrial = font;
    industrial.setTypefaceName ("DIN Condensed");
    industrial.setBold (true);
    return juce::Typeface::createSystemTypefaceFor (industrial);
}

void StepDistortLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                                float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                                juce::Slider&)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

    // Anchored to the top of the available bounds (not vertically centred),
    // so the knob stays glued to its label above even when its slot is much
    // taller than it is wide (e.g. after a vertical window resize).
    const juce::Point<float> centre { bounds.getCentreX(), bounds.getY() + radius };
    const float trackRadius = radius - 3.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Dim background track (the "unlit" arc).
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, trackRadius, trackRadius, 0.0f,
                          rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (emberDark);
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Fire-filled value arc, glowing hotter the further the knob is turned up.
    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, trackRadius, trackRadius, 0.0f,
                             rotaryStartAngle, angle, true);

    if (sliderPos > 0.02f)
    {
        juce::DropShadow glow (accentAlt.withAlpha (0.5f * sliderPos), (int) (5.0f + sliderPos * 12.0f), {});
        glow.drawForPath (g, valueArc);
    }

    juce::ColourGradient fireGrad (accentAlt, bounds.getX(), bounds.getBottom(),
                                    emberHot, bounds.getRight(), bounds.getY(), false);
    g.setGradientFill (fireGrad);
    g.strokePath (valueArc, juce::PathStrokeType (3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Dark brushed-metal knob body.
    auto knobBounds = juce::Rectangle<float> (radius * 1.3f, radius * 1.3f).withCentre (centre);
    juce::ColourGradient metal (juce::Colour (0xff332723), knobBounds.getTopLeft(),
                                 juce::Colour (0xff0b0807), knobBounds.getBottomRight(), false);
    g.setGradientFill (metal);
    g.fillEllipse (knobBounds);
    g.setColour (panelOutline);
    g.drawEllipse (knobBounds, 1.2f);

    // Glowing hot pointer showing the current value.
    juce::Path pointer;
    const float pointerLength = radius * 0.6f;
    const float pointerThickness = 2.6f;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength * 0.55f, 1.2f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    g.setColour (emberHot);
    g.fillPath (pointer);
}

void StepDistortLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                                bool, bool)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.5f);
    const bool on = button.getToggleState();

    if (on)
    {
        juce::Path shape;
        shape.addRoundedRectangle (bounds, 4.0f);
        juce::DropShadow glow (accentAlt.withAlpha (0.65f), 10, {});
        glow.drawForPath (g, shape);
    }

    juce::ColourGradient fill (on ? emberHot : juce::Colour (0xff251813), bounds.getX(), bounds.getY(),
                                on ? accentAlt : juce::Colour (0xff120b09), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (fill);
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (on ? emberHot.withAlpha (0.9f) : panelOutline);
    g.drawRoundedRectangle (bounds, 4.0f, on ? 1.6f : 1.0f);

    g.setColour (on ? juce::Colour (0xff2a0e04) : textDim);
    g.setFont (industrialFont (bounds.getHeight() * 0.62f));
    g.drawText (button.getButtonText(), bounds, juce::Justification::centred);
}

void StepDistortLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                            int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (1.0f);

    g.setColour (panel);
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (box.isPopupActive() ? accent : panelOutline);
    g.drawRoundedRectangle (bounds, 4.0f, 1.2f);

    auto arrowZone = bounds.removeFromRight (bounds.getHeight());
    juce::Path arrow;
    arrow.addTriangle (arrowZone.getCentreX() - 4.0f, arrowZone.getCentreY() - 2.5f,
                        arrowZone.getCentreX() + 4.0f, arrowZone.getCentreY() - 2.5f,
                        arrowZone.getCentreX(), arrowZone.getCentreY() + 3.5f);
    g.setColour (accent);
    g.fillPath (arrow);
}
