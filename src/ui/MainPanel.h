#pragma once

#include "../core/BrowseState.h"
#include "BrowserView.h"
#include "SettingsComponent.h"
#include "ToastNotification.h"
#include "OptionsComponent.h"

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

    using OptionsChanged = std::function<void(const AppOptions&)>;

    // Call once from the host after construction. Applies the options immediately,
    // then reports every later change through onChanged so the host can persist it.
    void configureOptions(const AppOptions& initial, OptionsChanged onChanged);

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

    enum class Tab { browse, options };
    void showTab(Tab tab);
    void applyOptions(const AppOptions& o);

    Tab currentTab = Tab::browse;
    juce::TextButton browseTabButton { "Browse" };
    juce::TextButton optionsTabButton { "Options" };
    OptionsComponent optionsPanel;
    OptionsChanged onOptionsChanged;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
};
}
