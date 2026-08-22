#pragma once

#include "../core/SamplePack.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace samplebox
{
class ArtworkCache;

class CoverArtCard final : public juce::Component
{
public:
    CoverArtCard(const SamplePack& pack, ArtworkCache& artworkCache);

    void paint(juce::Graphics& graphics) override;
    void setVisualState(float scale, float opacity, bool selected);

private:
    const SamplePack& pack;
    ArtworkCache& artworkCache;
    float scale = 1.0f;
    float opacity = 1.0f;
    bool selected = false;
};
}
