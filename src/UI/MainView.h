#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "../Data/DirectoryScanner.h"
#include "LookAndFeel.h"
#include "SettingsComponent.h"

namespace Antigravity
{
    // ==========================================================================
    // MainView
    //
    // The core UI panel shared between:
    //   - Standalone app  (hosted inside MainWindow / DocumentWindow)
    //   - VST3 plugin     (hosted inside PluginEditor / AudioProcessorEditor)
    //
    // It owns zero audio engine references — audio is wired separately by
    // each host (App or PluginProcessor). This keeps the UI completely
    // portable between the two targets.
    // ==========================================================================
    class MainView : public juce::Component,
                     public DirectoryScanner::Listener
    {
    public:
        // Standalone constructor — creates its own scanner
        MainView();

        // VST3 constructor — receives scanner from PluginProcessor
        explicit MainView(DirectoryScanner& externalScanner, const std::function<juce::String()>& getPath,
                          const std::function<void(const juce::String&)>& setPath);

        ~MainView() override;

        void paint(juce::Graphics&) override;
        void resized() override;

        // DirectoryScanner::Listener
        void scanProgressUpdated(float progress, int filesFound) override;
        void scanFinished(const std::vector<SampleFileInfo>& results) override;

        const std::vector<SampleFileInfo>& getIndexedFiles() const { return indexedFiles; }

    private:
        void init();

        // Scanner — either owned locally (Standalone) or borrowed (VST3)
        std::unique_ptr<DirectoryScanner> ownedScanner;
        DirectoryScanner* scanner = nullptr;

        // Persistent settings (Standalone mode only — VST3 uses PluginProcessor settings)
        std::unique_ptr<juce::PropertiesFile> standaloneSettings;
        std::function<juce::String()>          getLibraryPath;
        std::function<void(const juce::String&)> setLibraryPath;

        std::vector<SampleFileInfo> indexedFiles;

        // UI components
        LookAndFeel   customLookAndFeel;
        juce::Label   titleLabel;
        juce::Label   fileCountLabel;

        // Settings strip (Browse button + progress bar)
        // SettingsComponent needs access to getPath/setPath callbacks
        // so we forward them via lambdas bound at construction time
        std::unique_ptr<SettingsComponent> settingsStrip;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainView)
    };
}
