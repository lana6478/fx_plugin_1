#pragma once

#include <JuceHeader.h>

// The set of distortion algorithms a step can use.
enum class DistortionType
{
    clean = 0,
    softClip,
    hardClip,
    foldback,
    bitcrush
};

// Names shown in the UI dropdown, in the same order as DistortionType.
inline const juce::StringArray& getDistortionTypeNames()
{
    static const juce::StringArray names { "Clean", "Soft Clip", "Hard Clip", "Foldback", "Bitcrush" };
    return names;
}

// Applies one distortion algorithm to a single sample.
// `drive` is normalised 0..1 and is mapped to a useful range per algorithm.
inline float applyDistortion (float x, DistortionType type, float drive)
{
    switch (type)
    {
        case DistortionType::clean:
            return x;

        case DistortionType::softClip:
        {
            // tanh saturation: smooth, "warm" distortion. Normalised so max drive
            // doesn't just collapse everything to +-1.
            const float gain = 1.0f + drive * 19.0f;
            return std::tanh (x * gain) / std::tanh (gain);
        }

        case DistortionType::hardClip:
        {
            // Straight clamp: harsh, "digital" distortion.
            const float gain = 1.0f + drive * 9.0f;
            return juce::jlimit (-1.0f, 1.0f, x * gain);
        }

        case DistortionType::foldback:
        {
            // Reflects the signal back down every time it crosses +-1, instead of
            // clipping it flat. Gives a more chaotic, ring-mod-ish character at
            // high drive.
            const float gain = 1.0f + drive * 9.0f;
            float y = x * gain;
            const float threshold = 1.0f;

            // Bounded loop: gain is capped above, so this always converges quickly,
            // but we cap iterations anyway since this runs on the audio thread.
            for (int i = 0; i < 8 && std::abs (y) > threshold; ++i)
                y = (y > threshold) ? (2.0f * threshold - y) : (-2.0f * threshold - y);

            return y;
        }

        case DistortionType::bitcrush:
        {
            // Reduces bit depth: gritty, lo-fi distortion.
            const int bits = juce::jmax (1, 16 - (int) std::round (drive * 14.0f));
            const float levels = (float) (1 << bits);
            return std::round (x * levels) / levels;
        }

        default:
            return x;
    }
}
