#include "WaveformCache.h"

namespace samplebox
{
WaveformCache::WaveformCache(juce::AudioFormatManager& formatManagerToUse)
    : formatManager(formatManagerToUse)
{
}

juce::AudioThumbnail& WaveformCache::thumbnailFor(const std::filesystem::path& file)
{
    if (const auto found = thumbnails.find(file); found != thumbnails.end())
        return *found->second;

    auto thumbnail = std::make_unique<juce::AudioThumbnail>(512, formatManager, thumbnailCache);
    thumbnail->setSource(new juce::FileInputSource(juce::File(file.string())));

    auto& thumbnailRef = *thumbnail;
    thumbnails.emplace(file, std::move(thumbnail));
    return thumbnailRef;
}

void WaveformCache::clear()
{
    thumbnails.clear();
}
}
