#include "DistortionCurveDisplay.h"
#include "StepDistortLookAndFeel.h"

DistortionCurveDisplay::DistortionCurveDisplay (juce::AudioProcessorValueTreeState& stateToUse, int stepIndex)
    : apvts (stateToUse),
      typeParamID ("step" + juce::String (stepIndex) + "Type"),
      driveParamID ("step" + juce::String (stepIndex) + "Drive")
{
    apvts.addParameterListener (typeParamID, this);
    apvts.addParameterListener (driveParamID, this);
}

DistortionCurveDisplay::~DistortionCurveDisplay()
{
    apvts.removeParameterListener (typeParamID, this);
    apvts.removeParameterListener (driveParamID, this);
    cancelPendingUpdate();
}

void DistortionCurveDisplay::parameterChanged (const juce::String&, float)
{
    // May be called from the audio thread (e.g. host automation), so defer
    // the actual repaint to the message thread.
    triggerAsyncUpdate();
}

void DistortionCurveDisplay::handleAsyncUpdate()
{
    repaint();
}

void DistortionCurveDisplay::paint (juce::Graphics& g)
{
    using namespace StepDistortColours;

    auto bounds = getLocalBounds().toFloat();

    // Dark smouldering readout panel, slightly hotter toward the bottom.
    juce::ColourGradient panelGrad (juce::Colour (0xff140b09), bounds.getX(), bounds.getY(),
                                     juce::Colour (0xff1f0f0a), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (panelGrad);
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (panelOutline);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

    auto plotArea = bounds.reduced (5.0f);

    // Dim amber grid, like an old furnace-room gauge.
    g.setColour (emberDark.withAlpha (0.9f));
    g.drawHorizontalLine ((int) plotArea.getCentreY(), plotArea.getX(), plotArea.getRight());
    g.drawVerticalLine ((int) plotArea.getCentreX(), plotArea.getY(), plotArea.getBottom());

    const auto type = (DistortionType) (int) apvts.getRawParameterValue (typeParamID)->load();
    const auto drive = apvts.getRawParameterValue (driveParamID)->load();

    juce::Path curve;
    const int numPoints = 64;

    for (int i = 0; i <= numPoints; ++i)
    {
        const float x = (float) i / (float) numPoints * 2.0f - 1.0f; // -1..1
        const float y = juce::jlimit (-1.0f, 1.0f, applyDistortion (x, type, drive));

        const float px = plotArea.getX() + (x * 0.5f + 0.5f) * plotArea.getWidth();
        const float py = plotArea.getBottom() - (y * 0.5f + 0.5f) * plotArea.getHeight();

        if (i == 0)
            curve.startNewSubPath (px, py);
        else
            curve.lineTo (px, py);
    }

    // Soft glow behind the curve, then a crisp hot-metal gradient stroke on top.
    juce::DropShadow curveGlow (accentAlt.withAlpha (0.55f), 7, {});
    curveGlow.drawForPath (g, curve);

    juce::ColourGradient fireGrad (accentAlt, plotArea.getX(), 0.0f, emberHot, plotArea.getRight(), 0.0f, false);
    g.setGradientFill (fireGrad);
    g.strokePath (curve, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
}
