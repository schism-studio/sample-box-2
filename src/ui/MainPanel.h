#pragma once

#include "../core/BrowseState.h"
#include "BrowserView.h"
#include "SettingsComponent.h"

#if SAMPLEBOX_DRAG_SPIKE
#include "DragDropSpike.h"
#endif

#include <juce_gui_basics/juce_gui_basics.h>

#include <filesystem>
#include <functional>

namespace samplebox
{
// Shared UI composition hosted identically by the standalone MainWindow
// and the VST3 PluginEditor: a title header, the library-path/settings
// strip, and the sample-pack browser.
class MainPanel final : public juce::Component
{
public:
    using PlaySample = std::function<void(const std::filesystem::path&)>;

    MainPanel(SettingsComponent::GetPath getPath,
              SettingsComponent::SetPath setPath,
              PlaySample playSample = {});

    void setLibrary(LibrarySnapshotPtr snapshot);
    void setStatusText(const juce::String& text);

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    BrowseState browseState;
    juce::Label titleLabel;
    SettingsComponent settingsStrip;
    BrowserView browserView;

#if SAMPLEBOX_DRAG_SPIKE
    // TEMPORARY. Remove with src/ui/DragDropSpike.* — see that header.
    DragDropSpike dragSpike;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
};
}
