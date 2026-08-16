#pragma once

#include <JuceHeader.h>

// The set of distortion algorithms a step can use.
enum class DistortionType
{
    clean = 0,
    softClip,
    hardClip,
    foldback,
    bitcrush,
    saturation,
    overdrive,
    fuzz,
    tape,
    tube
};

// Names shown in the UI dropdown, in the same order as DistortionType.
inline const juce::StringArray& getDistortionTypeNames()
{
    static const juce::StringArray names { "Clean", "Soft Clip", "Hard Clip", "Foldback", "Bitcrush",
                                            "Saturation", "Overdrive", "Fuzz", "Tape", "Tube" };
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

        case DistortionType::saturation:
        {
            // Cubic soft-saturation: a smoother, rounder knee than the tanh
            // soft-clip above, closer to gentle analogue-console saturation.
            const float gain = 1.0f + drive * 11.0f;
            const float y = juce::jlimit (-1.0f, 1.0f, x * gain);
            return (y - (y * y * y) / 3.0f) * 1.5f;
        }

        case DistortionType::overdrive:
        {
            // Classic 3-segment cubic overdrive curve: stays clean at low
            // level, breaks up progressively, flattens smoothly at the top
            // (amp-like "breakup" rather than a hard knee).
            const float gain = 1.0f + drive * 5.0f;
            const float y = juce::jlimit (-1.0f, 1.0f, x * gain);
            const float ay = std::abs (y);
            const float sign = y >= 0.0f ? 1.0f : -1.0f;

            if (ay < 1.0f / 3.0f)
                return 2.0f * y;

            if (ay < 2.0f / 3.0f)
            {
                const float t = 2.0f - 3.0f * ay;
                return sign * (3.0f - t * t) / 3.0f;
            }

            return sign;
        }

        case DistortionType::fuzz:
        {
            // High-gain asymmetric clip (positive/negative thresholds
            // differ, like a germanium fuzz pedal) through an arctangent
            // knee for a spiky, harmonic-rich character.
            const float gain = 1.0f + drive * 39.0f;
            const float y = x * gain;
            const float clipped = (y >= 0.0f) ? juce::jmin (y, 1.0f) : juce::jmax (y, -0.85f);
            return std::atan (clipped * 3.0f) / std::atan (3.0f);
        }

        case DistortionType::tape:
        {
            // Asymmetric tanh saturation (tape bias saturates each half of
            // the waveform slightly differently) for a warmer, "glued"
            // character than the symmetric soft clip.
            const float gain = 1.0f + drive * 7.0f;
            const float driven = x * gain;
            const float shaped = (driven >= 0.0f) ? std::tanh (driven) : std::tanh (driven * 0.82f);
            return shaped / std::tanh (gain);
        }

        case DistortionType::tube:
        {
            // Single-ended exponential saturator: a rounder, softer knee
            // than tanh with a more gradual approach to full saturation,
            // in the spirit of classic tube-amp waveshaping.
            const float gain = 1.0f + drive * 9.0f;
            const float driven = x * gain;
            const float shaped = (driven >= 0.0f) ? (1.0f - std::exp (-driven)) : -(1.0f - std::exp (driven));
            return shaped / (1.0f - std::exp (-gain));
        }

        default:
            return x;
    }
}
