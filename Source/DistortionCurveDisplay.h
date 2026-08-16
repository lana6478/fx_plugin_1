#pragma once

#include <JuceHeader.h>
#include "Distortion.h"

// Plots y = applyDistortion(x, type, drive) for one step, so the step visually
// shows the shape of the distortion curve it's currently using. Repaints
// itself whenever that step's Type or Drive parameter changes.
class DistortionCurveDisplay : public juce::Component,
                                private juce::AudioProcessorValueTreeState::Listener,
                                private juce::AsyncUpdater
{
public:
    DistortionCurveDisplay (juce::AudioProcessorValueTreeState& apvts, int stepIndex);
    ~DistortionCurveDisplay() override;

    void paint (juce::Graphics&) override;

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void handleAsyncUpdate() override;

    juce::AudioProcessorValueTreeState& apvts;
    const juce::String typeParamID;
    const juce::String driveParamID;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionCurveDisplay)
};
