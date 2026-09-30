#pragma once

#include "../audio/PreviewEngine.h"
#include "../indexing/LibraryScanner.h"
#include "../ui/LookAndFeel.h"
#include "../ui/MainPanel.h"

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace samplebox
{
// Standalone application window. Owns the LibraryScanner and persisted
// settings, and hosts the same MainPanel used by the VST3 PluginEditor so
// both targets stay visually and behaviourally identical. Also owns a
// PreviewEngine driven through its own AudioDeviceManager, since (unlike
// the VST3) the standalone owns its audio device outright. See
// docs/decisions/0002-preview-audio-path.md.
class MainWindow final : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name);
    ~MainWindow() override;

    void closeButtonPressed() override;

    PreviewEngine& getPreviewEngine() { return previewEngine; }

private:
    juce::String getLibraryPath() const;
    void setLibraryPath(const juce::String& path);
    void startScan(const juce::File& root);

    LookAndFeel lookAndFeel;
    LibraryScanner scanner;
    std::unique_ptr<juce::PropertiesFile> settings;
    MainPanel mainPanel;

    PreviewEngine previewEngine;
    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};
}
