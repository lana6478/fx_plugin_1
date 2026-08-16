#pragma once

#include <JuceHeader.h>

// Shared value-display formatting for rotary sliders, so every knob shows a
// short, easy-to-read number instead of the raw floating point value.
namespace SliderFormatting
{
    // For 0..1 parameters that read best as a percentage (Drive, Mix, Smoothness).
    inline void configurePercentDisplay (juce::Slider& s)
    {
        s.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)) + "%"; };
        s.valueFromTextFunction = [] (const juce::String& t)
        {
            return t.retainCharacters ("0123456789.-").getDoubleValue() / 100.0;
        };
        s.setNumDecimalPlacesToDisplay (0);
    }

    // For the filter cutoff (whole Hz is plenty precise, and much easier to read).
    inline void configureHzDisplay (juce::Slider& s)
    {
        s.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v)) + " Hz"; };
        s.valueFromTextFunction = [] (const juce::String& t) { return t.retainCharacters ("0123456789.-").getDoubleValue(); };
        s.setNumDecimalPlacesToDisplay (0);
    }

    // For filter resonance (Q): one decimal place is enough to distinguish
    // useful settings without a long string of digits.
    inline void configureQDisplay (juce::Slider& s)
    {
        s.setNumDecimalPlacesToDisplay (1);
    }

    // For the output gain, in dB.
    inline void configureDbDisplay (juce::Slider& s)
    {
        s.textFromValueFunction = [] (double v) { return juce::String (v, 1) + " dB"; };
        s.valueFromTextFunction = [] (const juce::String& t) { return t.retainCharacters ("0123456789.-").getDoubleValue(); };
        s.setNumDecimalPlacesToDisplay (1);
    }
}
