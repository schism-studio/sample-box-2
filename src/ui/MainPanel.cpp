#include "MainPanel.h"
#include "Theme.h"

#include <utility>

namespace samplebox
{
MainPanel::MainPanel(SettingsComponent::GetPath getPath,
                     SettingsComponent::SetPath setPath,
                     PlaySample playSample,
                     StopPlayback stopPlayback)
    : MainPanel(defaultBrowseState, std::move(getPath), std::move(setPath), std::move(playSample), std::move(stopPlayback))
{
}

MainPanel::MainPanel(BrowseState& sharedBrowseState,
                     SettingsComponent::GetPath getPath,
                     SettingsComponent::SetPath setPath,
                     PlaySample playSample,
                     StopPlayback stopPlayback)
    : browseState(sharedBrowseState),
      onStopPlayback(std::move(stopPlayback)),
      settingsStrip(std::move(getPath), std::move(setPath)),
      browserView(browseState, std::move(playSample))
{
    setWantsKeyboardFocus(true);

    titleLabel.setText("Sample Box", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(22.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, theme::accent);
    addAndMakeVisible(titleLabel);

    stopButton.setButtonText(juce::CharPointer_UTF8("\xe2\x96\xa0 STOP"));
    stopButton.setColour(juce::TextButton::buttonColourId, theme::surface);
    stopButton.setColour(juce::TextButton::textColourOffId, theme::textSecondary);
    stopButton.setTooltip("Stop active sample preview (Space / Esc)");
    stopButton.onClick = [this] { triggerStop(); };
    addAndMakeVisible(stopButton);

    addAndMakeVisible(settingsStrip);
    addAndMakeVisible(browserView);
    addChildComponent(toastNotification);

#if SAMPLEBOX_DRAG_SPIKE
    addAndMakeVisible(dragSpike);
#endif
}

void MainPanel::setLibrary(LibrarySnapshotPtr snapshot)
{
    const auto packsCount = snapshot != nullptr ? snapshot->packs.size() : 0u;
    const auto samplesCount = snapshot != nullptr ? snapshot->tally.sampleFiles : 0u;

    browseState.setSnapshot(std::move(snapshot));
    browserView.refresh();

    if (packsCount > 0)
    {
        juce::String msg = juce::String((int) packsCount) + (packsCount == 1 ? " pack" : " packs")
                         + " \xe2\x80\xa2 " + juce::String((int) samplesCount) + (samplesCount == 1 ? " sample" : " samples") + " indexed";
        showToast(msg);
    }
}

void MainPanel::setStatusText(const juce::String& text)
{
    settingsStrip.setStatusText(text);
}

void MainPanel::showToast(const juce::String& message, int durationMs)
{
    toastNotification.showMessage(message, durationMs);
    toastNotification.toFront(true);
}

void MainPanel::triggerStop()
{
    if (onStopPlayback)
        onStopPlayback();

    browseState.clearSelection();
}

bool MainPanel::keyPressed(const juce::KeyPress& key)
{
    if (key.isKeyCode(juce::KeyPress::spaceKey) || key.isKeyCode(juce::KeyPress::escapeKey))
    {
        triggerStop();
        return true;
    }

    return Component::keyPressed(key);
}

void MainPanel::paint(juce::Graphics& graphics)
{
    graphics.fillAll(theme::background);
}

void MainPanel::resized()
{
    auto bounds = getLocalBounds();

    auto header = bounds.removeFromTop(48).reduced(14, 0);
    stopButton.setBounds(header.removeFromRight(76).reduced(0, 8));
    titleLabel.setBounds(header);

    settingsStrip.setBounds(bounds.removeFromTop(66).reduced(12, 6));

#if SAMPLEBOX_DRAG_SPIKE
    // Bottom-left corner, deliberately in the way. It is a diagnostic, and it
    // should be impossible to forget it is still compiled in.
    dragSpike.setBounds(bounds.removeFromBottom(96).removeFromLeft(280).reduced(12, 6));
#endif

    browserView.setBounds(bounds);

    // Center toast at bottom of main panel
    constexpr int toastWidth = 360;
    constexpr int toastHeight = 36;
    const int toastX = (getWidth() - toastWidth) / 2;
    const int toastY = getHeight() - toastHeight - 20;
    toastNotification.setBounds(toastX, toastY, toastWidth, toastHeight);
}
}
