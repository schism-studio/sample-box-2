#include "MainView.h"

namespace Antigravity
{
    // --------------------------------------------------------------------------
    // Standalone constructor — owns its own scanner and settings
    // --------------------------------------------------------------------------
    MainView::MainView()
    {
        // Create and own the scanner
        ownedScanner = std::make_unique<DirectoryScanner>();
        scanner = ownedScanner.get();

        // Set up persistent settings for standalone mode
        juce::PropertiesFile::Options opts;
        opts.applicationName     = "SamplePackVisualBrowser";
        opts.filenameSuffix      = "xml";
        opts.folderName          = "SamplePackVisualBrowser";
        opts.osxLibrarySubFolder = "Application Support";
        standaloneSettings       = std::make_unique<juce::PropertiesFile>(opts);

        getLibraryPath = [this]() -> juce::String {
            return standaloneSettings->getValue("sampleLibraryPath", "");
        };

        setLibraryPath = [this](const juce::String& path) {
            standaloneSettings->setValue("sampleLibraryPath", path);
            standaloneSettings->saveIfNeeded();
            juce::File root(path);
            if (root.isDirectory())
                scanner->startScan(root);
        };

        init();

        // Auto-resume scan from last session
        juce::String saved = getLibraryPath();
        if (saved.isNotEmpty())
        {
            juce::File root(saved);
            if (root.isDirectory())
                scanner->startScan(root);
        }
    }

    // --------------------------------------------------------------------------
    // VST3 constructor — receives scanner + path callbacks from PluginProcessor
    // --------------------------------------------------------------------------
    MainView::MainView(DirectoryScanner& externalScanner,
                       const std::function<juce::String()>& getPath,
                       const std::function<void(const juce::String&)>& setPath)
        : scanner(&externalScanner),
          getLibraryPath(getPath),
          setLibraryPath(setPath)
    {
        init();
    }

    MainView::~MainView()
    {
        if (scanner != nullptr)
            scanner->removeListener(this);
        setLookAndFeel(nullptr);
    }

    // --------------------------------------------------------------------------
    // Shared init
    // --------------------------------------------------------------------------
    void MainView::init()
    {
        setLookAndFeel(&customLookAndFeel);

        //---- Title -------------------------------------------------------
        titleLabel.setText("Sample Pack Visual Browser", juce::dontSendNotification);
        titleLabel.setFont(juce::Font(22.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centredLeft);
        titleLabel.setColour(juce::Label::textColourId, LookAndFeel::getAccentColor());
        addAndMakeVisible(titleLabel);

        //---- File count --------------------------------------------------
        fileCountLabel.setText("No samples indexed", juce::dontSendNotification);
        fileCountLabel.setFont(juce::Font(13.0f));
        fileCountLabel.setJustificationType(juce::Justification::centredRight);
        fileCountLabel.setColour(juce::Label::textColourId, LookAndFeel::getTextDimmedColor());
        addAndMakeVisible(fileCountLabel);

        //---- Settings strip ----------------------------------------------
        // SettingsComponent needs a PluginProcessor& in the current design.
        // For standalone use, we construct it with lambda-based forwarding
        // via the SettingsComponent constructor that accepts callbacks.
        // (This requires a small refactor of SettingsComponent — see below.)
        // For now, we build a lightweight inline strip directly here.

        //---- Subscribe to scanner ----------------------------------------
        if (scanner != nullptr)
            scanner->addListener(this);

        // Reflect existing indexed files (VST3 re-open)
        if (!indexedFiles.empty())
            fileCountLabel.setText(juce::String(indexedFiles.size()) + " samples indexed",
                                   juce::dontSendNotification);
    }

    // --------------------------------------------------------------------------
    void MainView::paint(juce::Graphics& g)
    {
        // Dark background
        g.fillAll(LookAndFeel::getDarkBackground());

        // Header separator
        g.setColour(LookAndFeel::getComponentBackground());
        g.fillRect(0, 54, getWidth(), 1);

        // Footer separator
        g.fillRect(0, getHeight() - 80, getWidth(), 1);

        // Content placeholder (M3: replaced by OpenGL carousel)
        auto contentArea = getLocalBounds().withTrimmedTop(56).withTrimmedBottom(82);
        g.setColour(LookAndFeel::getMidBackground());
        g.fillRect(contentArea);
        g.setColour(LookAndFeel::getTextDimmedColor());
        g.setFont(juce::Font(16.0f));
        g.drawText("M3: 3D Carousel renders here", contentArea, juce::Justification::centred);
    }

    void MainView::resized()
    {
        auto bounds = getLocalBounds();

        // Header
        auto header = bounds.removeFromTop(54).reduced(14, 0);
        titleLabel.setBounds(header.removeFromLeft(400));
        fileCountLabel.setBounds(header);

        // Footer settings strip (placeholder — full SettingsComponent wired in next step)
        bounds.removeFromBottom(80);
    }

    // --------------------------------------------------------------------------
    void MainView::scanProgressUpdated(float /*progress*/, int filesFound)
    {
        fileCountLabel.setText("Scanning... " + juce::String(filesFound) + " found",
                               juce::dontSendNotification);
    }

    void MainView::scanFinished(const std::vector<SampleFileInfo>& results)
    {
        indexedFiles = results;
        fileCountLabel.setText(juce::String(results.size()) + " samples indexed",
                               juce::dontSendNotification);
    }
}
