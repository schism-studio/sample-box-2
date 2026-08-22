#include "LookAndFeel.h"

namespace Antigravity
{
    LookAndFeel::LookAndFeel()
    {
        // Custom look and feel setup, e.g. slider colours, button styles, font overrides
        setColour(juce::ResizableWindow::backgroundColourId, getDarkBackground());
        setColour(juce::TextButton::buttonColourId, getComponentBackground());
        setColour(juce::TextButton::buttonOnColourId, getAccentColor());
        setColour(juce::TextButton::textColourOffId, getTextColor());
        setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        
        setColour(juce::Label::textColourId, getTextColor());
    }
}
