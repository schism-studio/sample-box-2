#pragma once

#include "../core/BrowseState.h"
#include "BrowserView.h"
#include "SettingsComponent.h"
#include "ToastNotification.h"

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
    using StopPlayback = std::function<void()>;

    MainPanel(SettingsComponent::GetPath getPath,
              SettingsComponent::SetPath setPath,
              PlaySample playSample = {},
              StopPlayback stopPlayback = {});

    MainPanel(BrowseState& sharedBrowseState,
              SettingsComponent::GetPath getPath,
              SettingsComponent::SetPath setPath,
              PlaySample playSample = {},
              StopPlayback stopPlayback = {});

    void setLibrary(LibrarySnapshotPtr snapshot);
    void setStatusText(const juce::String& text);
    void showToast(const juce::String& message, int durationMs = 3500);

    BrowseState& getBrowseState() { return browseState; }
    const BrowseState& getBrowseState() const { return browseState; }

    bool keyPressed(const juce::KeyPress& key) override;
    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void triggerStop();

    BrowseState defaultBrowseState;
    BrowseState& browseState;
    StopPlayback onStopPlayback;

    juce::Label titleLabel;
    juce::TextButton stopButton;
    SettingsComponent settingsStrip;
    BrowserView browserView;
    ToastNotification toastNotification;

#if SAMPLEBOX_DRAG_SPIKE
    // TEMPORARY. Remove with src/ui/DragDropSpike.* — see that header.
    DragDropSpike dragSpike;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
};
}
