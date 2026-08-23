#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <filesystem>
#include <memory>

namespace samplebox
{
// Single-voice, instant-choke sample preview player. Starting a new
// preview immediately stops whatever was previously playing, matching the
// click-free retrigger behaviour of the original TypeScript PreviewVoice
// prototype. Used by both the standalone app and the VST3 editor for the
// front-cover Play button audition path (unchanged by the VST3 pivot).
class PreviewEngine final : public juce::AudioSource
{
public:
    PreviewEngine();
    ~PreviewEngine() override;

    // Immediately chokes any currently-playing preview and starts playing
    // `file` from the start. Returns false if the file could not be opened.
    bool play(const std::filesystem::path& file);

    // Stops playback without starting a new preview.
    void stop();

    bool isPlaying() const;

    // juce::AudioSource
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

private:
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewEngine)
};
}
