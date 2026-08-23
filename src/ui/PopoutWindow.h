#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace samplebox
{
// A detached juce::DocumentWindow that can host any samplebox::Component
// (typically MainPanel), so the sample browser can "pop out" of the VST3
// editor into its own native window. Replaces the original Ableton-only
// "device rack vs. floating window" framing — this behaves identically in
// any VST3 host since the browser never reflects DAW-specific state.
class PopoutWindow final : public juce::DocumentWindow
{
public:
    // `contentToDisplay` is not owned by this window; the caller keeps it
    // alive and is responsible for destroying the window (e.g. from
    // `onClosed`) before the content is destroyed.
    PopoutWindow(const juce::String& name, juce::Component& contentToDisplay);

    void closeButtonPressed() override;

    std::function<void()> onClosed;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PopoutWindow)
};
}
