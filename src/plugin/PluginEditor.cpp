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
    processor.getScanner().scanAsync(
        std::filesystem::path(root.getFullPathName().toStdString()),
        [this](LibrarySnapshot snapshot) {
            const auto count = snapshot.packs.size();
            mainPanel.setLibrary(std::move(snapshot));
            mainPanel.setStatusText(juce::String((int) count) + " packs indexed");
        });
}
}
