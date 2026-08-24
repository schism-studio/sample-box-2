#include "MainWindow.h"
#include "../ui/Theme.h"

#include <filesystem>

namespace samplebox
{
namespace
{
juce::PropertiesFile::Options makeSettingsOptions()
{
    juce::PropertiesFile::Options opts;
    opts.applicationName = "SampleBox";
    opts.filenameSuffix = "xml";
    opts.folderName = "SampleBox";
    opts.osxLibrarySubFolder = "Application Support";
    return opts;
}
}

MainWindow::MainWindow(const juce::String& name)
    : DocumentWindow(name, theme::background, DocumentWindow::allButtons),
      settings(std::make_unique<juce::PropertiesFile>(makeSettingsOptions())),
      mainPanel([this] { return getLibraryPath(); },
                 [this](const juce::String& path) { setLibraryPath(path); })
{
    setLookAndFeel(&lookAndFeel);
    setUsingNativeTitleBar(true);
    setResizable(true, false);
    setResizeLimits(700, 450, 2400, 1600);

    setContentNonOwned(&mainPanel, false);
    mainPanel.setSize(1200, 760);

    centreWithSize(1200, 760);
    setVisible(true);

    const auto savedPath = getLibraryPath();
    if (savedPath.isNotEmpty())
        startScan(juce::File(savedPath));
}

MainWindow::~MainWindow()
{
    setLookAndFeel(nullptr);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

juce::String MainWindow::getLibraryPath() const
{
    return settings->getValue("sampleLibraryPath", "");
}

void MainWindow::setLibraryPath(const juce::String& path)
{
    settings->setValue("sampleLibraryPath", path);
    settings->saveIfNeeded();
    startScan(juce::File(path));
}

void MainWindow::startScan(const juce::File& root)
{
    if (!root.isDirectory())
        return;

    mainPanel.setStatusText("Scanning...");
    scanner.scanAsync(
        std::filesystem::path(root.getFullPathName().toStdString()),
        [this](LibrarySnapshotPtr snapshot) {
            const auto count = snapshot != nullptr ? snapshot->packs.size() : 0u;
            mainPanel.setLibrary(std::move(snapshot));
            mainPanel.setStatusText(juce::String((int) count) + " packs indexed");
        });
}
}
