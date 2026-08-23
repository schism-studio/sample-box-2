#pragma once

#include "PluginProcessor.h"
#include "../ui/LookAndFeel.h"
#include "../ui/MainPanel.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace samplebox
{
// VST3 editor. Hosts the same MainPanel used by the standalone
// MainWindow, and borrows its LibraryScanner from the processor rather
// than owning its own — the scanner's lifetime must match the plugin
// instance, not the editor (which a DAW may close and reopen freely).
class PluginEditor final : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor(PluginProcessor&);
    ~PluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void startScan(const juce::File& root);

    PluginProcessor& processor;
    LookAndFeel lookAndFeel;
    MainPanel mainPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
}
