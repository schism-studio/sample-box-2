#pragma once

#include "Theme.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace samplebox
{
// Sleek, floating notification toast that displays scan completion metrics
// (packs & total samples indexed) or cancellation messages, and fades out automatically.
class ToastNotification final : public juce::Component,
                                private juce::Timer
{
public:
    ToastNotification()
    {
        setInterceptsMouseClicks(false, false);
        setAlwaysOnTop(true);
    }

    ~ToastNotification() override
    {
        stopTimer();
    }

    void showMessage(const juce::String& text, int durationMs = 3500)
    {
        messageText = text;
        opacity = 1.0f;
        setVisible(true);
        repaint();

        stopTimer();
        fadeStartTime = juce::Time::getMillisecondCounter();
        displayDurationMs = durationMs;
        startTimerHz(60);
    }

    void paint(juce::Graphics& graphics) override
    {
        if (opacity <= 0.001f || messageText.isEmpty())
            return;

        const auto bounds = getLocalBounds().toFloat();

        graphics.setOpacity(opacity);

        // Subtle dark glass surface with accent border
        graphics.setColour(juce::Colour(0xee1b1d22));
        graphics.fillRoundedRectangle(bounds, 8.0f);

        graphics.setColour(theme::accent.withAlpha(0.6f * opacity));
        graphics.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

        graphics.setColour(theme::textPrimary.withAlpha(opacity));
        graphics.setFont(juce::Font(14.0f, juce::Font::bold));
        graphics.drawFittedText(messageText, getLocalBounds().reduced(16, 6), juce::Justification::centred, 1);
    }

private:
    void timerCallback() override
    {
        const auto elapsed = static_cast<int>(juce::Time::getMillisecondCounter() - fadeStartTime);

        if (elapsed > displayDurationMs)
        {
            constexpr int fadeDurationMs = 400;
            const auto fadeProgress = static_cast<float>(elapsed - displayDurationMs) / static_cast<float>(fadeDurationMs);

            if (fadeProgress >= 1.0f)
            {
                opacity = 0.0f;
                setVisible(false);
                stopTimer();
            }
            else
            {
                opacity = 1.0f - fadeProgress;
                repaint();
            }
        }
    }

    juce::String messageText;
    float opacity = 0.0f;
    std::uint32_t fadeStartTime = 0;
    int displayDurationMs = 3500;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToastNotification)
};
}
