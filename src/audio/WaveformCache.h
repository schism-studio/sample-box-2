#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <filesystem>
#include <map>
#include <memory>

namespace samplebox
{
// Caches a juce::AudioThumbnail per sample file so waveform badges can be
// drawn cheaply once a file has been previewed or selected once. Shares a
// single thumbnail cache across all files rather than one per-component.
class WaveformCache final
{
public:
    explicit WaveformCache(juce::AudioFormatManager& formatManagerToUse);

    juce::AudioThumbnail& thumbnailFor(const std::filesystem::path& file);
    void clear();

private:
    juce::AudioFormatManager& formatManager;
    juce::AudioThumbnailCache thumbnailCache { 32 };
    std::map<std::filesystem::path, std::unique_ptr<juce::AudioThumbnail>> thumbnails;
};
}
