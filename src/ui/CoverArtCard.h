#pragma once

#include "../core/LibrarySnapshot.h"
#include "../core/SamplePack.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <cstddef>
#include <functional>

namespace samplebox
{
class ArtworkCache;

// Reusable card component for virtualized carousels and grid views.
// Can be dynamically rebound to different packs as scrolling moves.
class CoverArtCard final : public juce::Component
{
public:
    CoverArtCard(ArtworkCache& artworkCache,
                 std::function<void(std::size_t)> onPackClicked);

    void bindToPack(LibrarySnapshotPtr snapshot, std::size_t indexOfPack);

    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent&) override;
    void setVisualState(float scale, float opacity, bool selected);

    [[nodiscard]] std::size_t getPackIndex() const noexcept { return packIndex; }
    [[nodiscard]] bool hasValidPack() const noexcept
    {
        return librarySnapshot != nullptr && packIndex < librarySnapshot->packs.size();
    }

    [[nodiscard]] const SamplePack* getPack() const
    {
        if (hasValidPack())
            return &librarySnapshot->packs[packIndex];
        return nullptr;
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
