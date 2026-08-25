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

    // Preview audio: the standalone owns its device outright, so the engine
    // is driven directly, unlike the VST3 which mixes into processBlock.
    // addAudioCallback must precede setSource - AudioSourcePlayer only calls
    // prepareToPlay immediately if it already knows the sample rate, which it
    // only learns once the callback is registered and the device starts.
    audioDeviceManager.initialiseWithDefaultDevices(0, 2);
    audioDeviceManager.addAudioCallback(&audioSourcePlayer);
    audioSourcePlayer.setSource(&previewEngine);

    const auto savedPath = getLibraryPath();
    if (savedPath.isNotEmpty())
        startScan(juce::File(savedPath));
}

MainWindow::~MainWindow()
{
    audioSourcePlayer.setSource(nullptr);
    audioDeviceManager.removeAudioCallback(&audioSourcePlayer);
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
