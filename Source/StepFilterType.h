#pragma once

#include <JuceHeader.h>

// The filter a step can apply after its distortion stage.
enum class StepFilterType
{
    off = 0,
    lowpass,
    highpass,
    bandpass
};

// Names shown in the UI dropdown, in the same order as StepFilterType.
inline const juce::StringArray& getStepFilterTypeNames()
{
    static const juce::StringArray names { "Off", "Low Pass", "High Pass", "Band Pass" };
    return names;
}
