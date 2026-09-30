#pragma once

#include "../core/LibrarySnapshot.h"
#include "../core/SamplePack.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <cstddef>
#include <functional>

namespace samplebox
{
class ArtworkCache;

class CoverArtCard final : public juce::Component
{
public:
    CoverArtCard(LibrarySnapshotPtr snapshot,
                 std::size_t packIndex,
                 ArtworkCache& artworkCache,
                 std::function<void(std::size_t)> onPackClicked);

    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent&) override;
    void setVisualState(float scale, float opacity, bool selected);

    [[nodiscard]] const SamplePack& getPack() const
    {
        return librarySnapshot->packs[packIndex];
    }

private:
    LibrarySnapshotPtr librarySnapshot;
    std::size_t packIndex = 0;
    ArtworkCache& artworkCache;
    std::function<void(std::size_t)> onPackClicked;
    float scale = 1.0f;
    float opacity = 1.0f;
    bool selected = false;
};
}
