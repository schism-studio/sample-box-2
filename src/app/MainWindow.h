#pragma once

#include "../indexing/LibraryScanner.h"
#include "../ui/LookAndFeel.h"
#include "../ui/MainPanel.h"

#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace samplebox
{
// Standalone application window. Owns the LibraryScanner and persisted
// settings, and hosts the same MainPanel used by the VST3 PluginEditor so
// both targets stay visually and behaviourally identical.
class MainWindow final : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name);
    ~MainWindow() override;

    void closeButtonPressed() override;

private:
    juce::String getLibraryPath() const;
    void setLibraryPath(const juce::String& path);
    void startScan(const juce::File& root);

    LookAndFeel lookAndFeel;
    LibraryScanner scanner;
    std::unique_ptr<juce::PropertiesFile> settings;
    MainPanel mainPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};
}
