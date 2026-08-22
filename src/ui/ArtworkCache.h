#pragma once

#include <juce_graphics/juce_graphics.h>

#include <filesystem>
#include <map>

namespace samplebox
{
class ArtworkCache
{
public:
    [[nodiscard]] juce::Image imageFor(const std::filesystem::path& path);
    void clear();

private:
    std::map<std::filesystem::path, juce::Image> images;
};
}
