#include "PluginEditor.h"
#include "../ui/Theme.h"

#include <filesystem>

namespace samplebox
{
PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      mainPanel([&p] { return p.getSampleLibraryPath(); },
                 [this](const juce::String& path) {
                     processor.setSampleLibraryPath(path);
                     startScan(juce::File(path));
                 })
{
    setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(mainPanel);

    setResizable(true, true);
    setResizeLimits(700, 450, 2400, 1600);
    setSize(1200, 760);

    const auto savedPath = processor.getSampleLibraryPath();
    if (savedPath.isNotEmpty())
        startScan(juce::File(savedPath));
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel(nullptr);
}

void PluginEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(theme::background);
}

void PluginEditor::resized()
{
    mainPanel.setBounds(getLocalBounds());
}

void PluginEditor::startScan(const juce::File& root)
{
    if (!root.isDirectory())
        return;

    mainPanel.setStatusText("Scanning...");

    // The scanner is owned by the *processor*, which outlives this editor: a
    // DAW may close and reopen the plugin window freely, and it is allowed to
    // do so while a scan of a large library is still running. Capturing a raw
    // `this` here would leave the completion callback writing into a destroyed
    // editor. A SafePointer is nulled automatically when the Component dies,
    // and is safe to test here because the scanner always delivers its
    // completion on the message thread via MessageManager::callAsync.
    processor.getScanner().scanAsync(
        std::filesystem::path(root.getFullPathName().toStdString()),
        [safeEditor = juce::Component::SafePointer<PluginEditor>(this)](LibrarySnapshot snapshot) {
            if (safeEditor == nullptr)
                return;

            const auto count = snapshot.packs.size();
            safeEditor->mainPanel.setLibrary(std::move(snapshot));
            safeEditor->mainPanel.setStatusText(juce::String((int) count) + " packs indexed");
        });
}
}
