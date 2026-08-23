#pragma once

#include "../core/LibrarySnapshot.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace samplebox
{
// Flat, non-reactive list of the packs in the current library. Unlike the
// original Ableton-extension "SidebarTree", this view never has to mirror
// any DAW's own sample library state — per the VST3 export redesign,
// sample libraries do not need to stay synchronized across hosts, so this
// is intentionally just a plain browser.
class SidebarBrowser final : public juce::Component,
                              private juce::ListBoxModel
{
public:
    SidebarBrowser();

    void setLibrary(LibrarySnapshot snapshot);
    void resized() override;

    std::function<void(const SamplePack&)> onPackSelected;

private:
    // juce::ListBoxModel
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& graphics, int width, int height,
                           bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;

    LibrarySnapshot library;
    juce::ListBox listBox { "SidebarBrowser", this };
};
}
