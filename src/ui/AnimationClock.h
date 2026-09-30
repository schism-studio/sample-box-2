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

    // Starting an already-running clock is a no-op rather than a delta reset,
    // so callers can call this freely whenever they queue new motion.
    void start(int framesPerSecond = 60);
    void stop();

    [[nodiscard]] bool isRunning() const { return isTimerRunning(); }

private:
    void timerCallback() override;

    Tick onTick;
    double lastTickSeconds = 0.0;
};
}
