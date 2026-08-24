#pragma once

#include "../core/LibrarySnapshot.h"
#include "BrowserView.h"
#include "SettingsComponent.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace samplebox
{
// Shared UI composition hosted identically by the standalone MainWindow
// and the VST3 PluginEditor: a title header, the library-path/settings
// strip, and the cover-art browser. Keeping this in one place is what
// keeps the standalone and VST3 editors pixel- and behaviour-identical.
class MainPanel final : public juce::Component
{
public:
    MainPanel(SettingsComponent::GetPath getPath, SettingsComponent::SetPath setPath);

    void setLibrary(LibrarySnapshotPtr snapshot);
    void setStatusText(const juce::String& text);

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    juce::Label titleLabel;
    SettingsComponent settingsStrip;
    BrowserView browserView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
};
}
