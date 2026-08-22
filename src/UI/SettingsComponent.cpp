#include "SettingsComponent.h"

namespace Antigravity
{
    SettingsComponent::SettingsComponent(PluginProcessor& p)
        : processor(p), progressBar(scanProgress)
    {
        // Load initial path from processor configuration
        juce::String currentPath = processor.getSampleLibraryPath();
        pathLabel.setText(currentPath.isEmpty() ? "No path selected" : currentPath, juce::dontSendNotification);
        pathLabel.setColour(juce::Label::backgroundColourId, LookAndFeel::getMidBackground());
        pathLabel.setColour(juce::Label::outlineColourId, LookAndFeel::getComponentBackground());
        pathLabel.setBorderSize(juce::BorderSize<int>(4, 8, 4, 8));
        addAndMakeVisible(pathLabel);

        // Browse button
        browseButton.setButtonText("Browse...");
        browseButton.onClick = [this]() { selectButtonClicked(); };
        addAndMakeVisible(browseButton);

        // Progress indicators
        statusLabel.setText("Scanner idle", juce::dontSendNotification);
        statusLabel.setFont(juce::Font(13.0f));
        statusLabel.setColour(juce::Label::textColourId, LookAndFeel::getTextDimmedColor());
        addAndMakeVisible(statusLabel);

        progressBar.setPercentageDisplay(true);
        addAndMakeVisible(progressBar);

        // Subscribe to scanner events
        processor.getScanner().addListener(this);
    }

    SettingsComponent::~SettingsComponent()
    {
        processor.getScanner().removeListener(this);
    }

    void SettingsComponent::paint(juce::Graphics& g)
    {
        // Draw bottom dashboard frame border
        g.setColour(LookAndFeel::getMidBackground());
        g.fillAll();
    }

    void SettingsComponent::resized()
    {
        auto bounds = getLocalBounds().reduced(10);
        
        auto topRow = bounds.removeFromTop(30);
        browseButton.setBounds(topRow.removeFromRight(100));
        topRow.removeFromRight(10); // spacing
        pathLabel.setBounds(topRow);

        bounds.removeFromTop(10); // spacing
        
        auto bottomRow = bounds.removeFromTop(20);
        progressBar.setBounds(bottomRow.removeFromRight(200));
        bottomRow.removeFromRight(10); // spacing
        statusLabel.setBounds(bottomRow);
    }

    void SettingsComponent::selectButtonClicked()
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select Sample Library Location...",
            juce::File::getSpecialLocation(juce::File::userHomeDirectory),
            "*"
        );

        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;

        fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser) {
            auto result = chooser.getResult();
            if (result.exists())
            {
                juce::String chosenPath = result.getFullPathName();
                pathLabel.setText(chosenPath, juce::dontSendNotification);
                
                // Save to persistent file storage
                processor.setSampleLibraryPath(chosenPath);
            }
        });
    }

    void SettingsComponent::scanProgressUpdated(float progress, int filesFound)
    {
        scanProgress = progress;
        statusLabel.setText("Scanning: " + juce::String(filesFound) + " files found", juce::dontSendNotification);
    }

    void SettingsComponent::scanFinished(const std::vector<SampleFileInfo>& results)
    {
        scanProgress = 1.0;
        statusLabel.setText("Scan complete! " + juce::String(results.size()) + " samples indexed.", juce::dontSendNotification);
        processor.updateIndexedFiles(results);
    }
}
