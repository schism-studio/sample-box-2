#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../AudioEngine/PluginProcessor.h"
#include "LookAndFeel.h"
#include "SettingsComponent.h"

namespace Antigravity
{
    class PluginEditor : public juce::AudioProcessorEditor,
                         public DirectoryScanner::Listener
    {
    public:
        explicit PluginEditor(PluginProcessor&);
        ~PluginEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

        // DirectoryScanner::Listener — update status label from GUI thread
        void scanProgressUpdated(float progress, int filesFound) override;
        void scanFinished(const std::vector<SampleFileInfo>& results) override;

    private:
        PluginProcessor& processor;

        LookAndFeel            customLookAndFeel;

        // Header bar
        juce::Label            titleLabel;
        juce::Label            fileCountLabel;

        // Settings strip (path chooser + progress bar)
        SettingsComponent      settingsComponent;

        // M3: CarouselComponent + SidebarTree will be added here
        // M4: WaveformBadgeComponent + DragDropHandler will be added here

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
    };
}
