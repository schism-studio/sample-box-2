#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace Antigravity
{
    class LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        LookAndFeel();
        ~LookAndFeel() override = default;

        // Custom styling helper definitions
        static juce::Colour getDarkBackground()     { return juce::Colour::fromString("#FF181818"); }
        static juce::Colour getMidBackground()      { return juce::Colour::fromString("#FF242424"); }
        static juce::Colour getComponentBackground(){ return juce::Colour::fromString("#FF2D2D2D"); }
        static juce::Colour getAccentColor()        { return juce::Colour::fromString("#FFF38A00"); } // Ableton-style orange
        static juce::Colour getTextColor()          { return juce::Colour::fromString("#FFE3E3E3"); }
        static juce::Colour getTextDimmedColor()    { return juce::Colour::fromString("#FF8C8C8C"); }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LookAndFeel)
    };
}
