#include "AnimationClock.h"

#include <juce_core/juce_core.h>

namespace samplebox
{
AnimationClock::AnimationClock(Tick tick) : onTick(std::move(tick)) {}

void AnimationClock::start(int framesPerSecond)
{
    if (isTimerRunning())
        return;

    lastTickSeconds = juce::Time::getMillisecondCounterHiRes() * 0.001;
    startTimerHz(framesPerSecond);
}

void AnimationClock::stop()
{
    stopTimer();
}

void AnimationClock::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const auto delta = now - lastTickSeconds;
    lastTickSeconds = now;

    if (onTick)
        onTick(delta);
}
}
