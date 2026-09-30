#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <atomic>
#include <filesystem>
#include <memory>

namespace samplebox
{
// Single-voice sample preview player with click-free 5ms fade-out/fade-in
// smoothing on choke and stop. Prevents pops and abrupt discontinuities
// during auditioning.
class PreviewEngine final : public juce::AudioSource
{
public:
    PreviewEngine();
    ~PreviewEngine() override;

    // Smoothly chokes any active preview and starts playing `file` from the start.
    // Returns false if the file could not be opened.
    bool play(const std::filesystem::path& file);

    // Triggers a click-free 5ms fade-out stop without starting a new preview.
    void stop();

    bool isPlaying() const;

    // juce::AudioSource
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

private:
    void applyGainRamping(const juce::AudioSourceChannelInfo& bufferToFill);

    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    double currentSampleRate = 44100.0;
    std::atomic<bool> isFadingOut { false };
    float currentGain = 1.0f;
    float fadeOutStep = 0.01f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewEngine)
};
}
