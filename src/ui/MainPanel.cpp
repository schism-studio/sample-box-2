#include "MainPanel.h"
#include "Theme.h"

namespace samplebox
{
MainPanel::MainPanel(SettingsComponent::GetPath getPath, SettingsComponent::SetPath setPath)
    : settingsStrip(std::move(getPath), std::move(setPath))
{
    titleLabel.setText("Sample Box", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(22.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, theme::accent);
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(settingsStrip);
    addAndMakeVisible(browserView);

#if SAMPLEBOX_DRAG_SPIKE
    addAndMakeVisible(dragSpike);
#endif
}

void MainPanel::setLibrary(LibrarySnapshotPtr snapshot)
{
    browserView.setLibrary(std::move(snapshot));
}

void MainPanel::setStatusText(const juce::String& text)
{
    settingsStrip.setStatusText(text);
}

void MainPanel::paint(juce::Graphics& graphics)
{
    graphics.fillAll(theme::background);
}

void MainPanel::resized()
{
    auto bounds = getLocalBounds();

    auto header = bounds.removeFromTop(48).reduced(14, 0);
    titleLabel.setBounds(header);

    settingsStrip.setBounds(bounds.removeFromTop(66).reduced(12, 6));

#if SAMPLEBOX_DRAG_SPIKE
    // Bottom-left corner, deliberately in the way. It is a diagnostic, and it
    // should be impossible to forget it is still compiled in.
    dragSpike.setBounds(bounds.removeFromBottom(96).removeFromLeft(280).reduced(12, 6));
#endif

    browserView.setBounds(bounds);
}
}
