#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>

namespace samplebox
{
// Library-path chooser + status strip, shared by the standalone
// MainWindow and the VST3 PluginEditor via MainPanel. Deliberately has no
// reference to juce::AudioProcessor or LibraryScanner — the owner wires it
// up with plain callbacks, which is what lets the same component be used
// unmodified in both hosts.
class SettingsComponent final : public juce::Component
{
public:
    using GetPath = std::function<juce::String()>;
    using SetPath = std::function<void(const juce::String&)>;

    SettingsComponent(GetPath getPath, SetPath setPath);
    ~SettingsComponent() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

    // Called by the owner to reflect scan lifecycle, e.g. "Scanning..." or
    // "128 packs indexed".
    void setStatusText(const juce::String& text);

private:
    void browseButtonClicked();

    GetPath getLibraryPath;
    SetPath setLibraryPath;

    juce::Label pathLabel;
    juce::TextButton browseButton { "Browse..." };
    juce::Label statusLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};
}
