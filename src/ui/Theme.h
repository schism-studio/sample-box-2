#pragma once

#include <juce_graphics/juce_graphics.h>

namespace samplebox::theme
{
inline const juce::Colour background { 0xff101114 };
inline const juce::Colour surface { 0xff1b1d22 };
inline const juce::Colour textPrimary { 0xfff4f5f7 };
inline const juce::Colour textSecondary { 0xff9ca3af };
inline const juce::Colour accent { 0xff8b5cf6 };

inline constexpr float cardCornerRadius = 14.0f;
inline constexpr int carouselCardWidth = 240;
inline constexpr int carouselCardHeight = 300;
}
