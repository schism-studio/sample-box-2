#include "SettingsComponent.h"
#include "Theme.h"

namespace samplebox
{
SettingsComponent::SettingsComponent(GetPath getPath, SetPath setPath)
    : getLibraryPath(std::move(getPath)), setLibraryPath(std::move(setPath))
{
    const auto currentPath = getLibraryPath ? getLibraryPath() : juce::String();
    pathLabel.setText(currentPath.isEmpty() ? "No library folder selected" : currentPath,
                       juce::dontSendNotification);
    pathLabel.setColour(juce::Label::backgroundColourId, theme::surface);
    pathLabel.setColour(juce::Label::textColourId, theme::textPrimary);
    pathLabel.setBorderSize(juce::BorderSize<int>(4, 8, 4, 8));
    addAndMakeVisible(pathLabel);

    browseButton.onClick = [this] { browseButtonClicked(); };
    addAndMakeVisible(browseButton);

    statusLabel.setText("Idle", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(13.0f));
    statusLabel.setColour(juce::Label::textColourId, theme::textSecondary);
    addAndMakeVisible(statusLabel);
}

SettingsComponent::~SettingsComponent() = default;

void SettingsComponent::paint(juce::Graphics& graphics)
{
    graphics.fillAll(theme::surface);
}

void SettingsComponent::resized()
{
    auto bounds = getLocalBounds().reduced(8);

    auto topRow = bounds.removeFromTop(28);
    browseButton.setBounds(topRow.removeFromRight(100));
    topRow.removeFromRight(8);
    pathLabel.setBounds(topRow);

    bounds.removeFromTop(6);
    statusLabel.setBounds(bounds.removeFromTop(20));
}

void SettingsComponent::setStatusText(const juce::String& text)
{
    statusLabel.setText(text, juce::dontSendNotification);
}

void SettingsComponent::browseButtonClicked()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select Sample Library Folder...",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory));

    constexpr auto flags = juce::FileBrowserComponent::openMode
                            | juce::FileBrowserComponent::canSelectDirectories;

    fileChooser->launchAsync(flags, [this](const juce::FileChooser& chooser) {
        const auto result = chooser.getResult();
        if (!result.exists())
            return;

        const auto chosenPath = result.getFullPathName();
        pathLabel.setText(chosenPath, juce::dontSendNotification);

        if (setLibraryPath)
            setLibraryPath(chosenPath);
    });
}
}
