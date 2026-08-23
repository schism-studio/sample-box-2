#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace samplebox
{
// Shared dark theme applied to both the standalone window and the VST3
// editor. Colour values come from ui/Theme.h so widget styling always
// matches the carousel/browser palette. Ported from the original
// Antigravity look-and-feel prototype.
class LookAndFeel final : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();
    ~LookAndFeel() override = default;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LookAndFeel)
};
}
