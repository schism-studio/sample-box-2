#pragma once

#include <juce_events/juce_events.h>

#include <functional>

namespace samplebox
{
class AnimationClock final : private juce::Timer
{
public:
    using Tick = std::function<void(double deltaSeconds)>;

    explicit AnimationClock(Tick tick);
    void start(int framesPerSecond = 60);
    void stop();

private:
    void timerCallback() override;

    Tick onTick;
    double lastTickSeconds = 0.0;
};
}
