#pragma once

#include "../AudioEngine/PluginProcessor.h"
#include "../Data/DirectoryScanner.h"
#include "LookAndFeel.h"

namespace Antigravity
{
    class SettingsComponent : public juce::Component,
                              public DirectoryScanner::Listener
    {
    public:
        SettingsComponent(PluginProcessor&);
        ~SettingsComponent() override;

        void paint(juce::Graphics&) override;
        void resized() override;

        // DirectoryScanner::Listener overrides
        void scanProgressUpdated(float progress, int filesFound) override;
        void scanFinished(const std::vector<SampleFileInfo>& results) override;

    private:
        PluginProcessor& processor;

        juce::Label pathLabel;
        juce::TextButton browseButton;
        juce::Label statusLabel;
        juce::ProgressBar progressBar;
        
        double scanProgress = 0.0;
        std::unique_ptr<juce::FileChooser> fileChooser;

        void selectButtonClicked();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
    };
}
