#include "LookAndFeel.h"
#include "Theme.h"

namespace samplebox
{
LookAndFeel::LookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, theme::background);

    setColour(juce::TextButton::buttonColourId, theme::surface);
    setColour(juce::TextButton::buttonOnColourId, theme::accent);
    setColour(juce::TextButton::textColourOffId, theme::textPrimary);
    setColour(juce::TextButton::textColourOnId, theme::background);

    setColour(juce::Label::textColourId, theme::textPrimary);
    setColour(juce::Label::backgroundColourId, theme::surface);
    setColour(juce::Label::outlineColourId, theme::surface);

    setColour(juce::ProgressBar::backgroundColourId, theme::surface);
    setColour(juce::ProgressBar::foregroundColourId, theme::accent);

    setColour(juce::ScrollBar::thumbColourId, theme::textSecondary);
    setColour(juce::ListBox::backgroundColourId, theme::background);
    setColour(juce::ListBox::textColourId, theme::textPrimary);
}
}
