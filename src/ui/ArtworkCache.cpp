#include "ArtworkCache.h"

#include <juce_graphics/juce_graphics.h>

namespace samplebox
{
juce::Image ArtworkCache::imageFor(const std::filesystem::path& path)
{
    if (path.empty())
        return {};

    if (const auto found = images.find(path); found != images.end())
        return found->second;

    const auto image = juce::ImageFileFormat::loadFrom(juce::File(path.string()));
    images.emplace(path, image);
    return image;
}

void ArtworkCache::clear()
{
    images.clear();
}
}
