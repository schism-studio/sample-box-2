#pragma once

#include "LibrarySnapshot.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace samplebox
{
enum class BrowseLevel
{
    packs,
    samples
};

enum class PackViewMode
{
    list,
    grid,
    coverFlow
};

// Shared, host-independent browser state. MainPanel owns this so standalone
// and VST3 use one snapshot, focus, selection, filters, and view mode.
struct BrowseState
{
    BrowseLevel level = BrowseLevel::packs;
    PackViewMode packViewMode = PackViewMode::coverFlow;

    LibrarySnapshotPtr snapshot;
    std::string focusedPackId;
    std::filesystem::path selectedSamplePath;
    std::string searchQuery;
    std::vector<std::string> extensionFilter;

    // Continuous zoom per view mode, 0.0–1.0. Each view maps this to its own
    // pixel range. Stored here so the user's zoom choice survives view switches.
    double thumbnailZoomGrid = 0.5;
    double thumbnailZoomList = 0.5;
    double thumbnailZoomCoverFlow = 0.5;

    void setSnapshot(LibrarySnapshotPtr newSnapshot)
    {
        snapshot = std::move(newSnapshot);
        level = BrowseLevel::packs;
        focusedPackId.clear();
        selectedSamplePath.clear();
    }

    [[nodiscard]] const SamplePack* findFocusedPack() const
    {
        if (snapshot == nullptr || focusedPackId.empty())
            return nullptr;

        const auto matchingPack = std::find_if(snapshot->packs.begin(),
                                               snapshot->packs.end(),
                                               [this](const SamplePack& pack)
                                               {
                                                   return pack.id == focusedPackId;
                                               });

        return matchingPack != snapshot->packs.end() ? &*matchingPack : nullptr;
    }

    [[nodiscard]] bool hasFocusedPack() const
    {
        return findFocusedPack() != nullptr;
    }

    void focusPack(const SamplePack& pack)
    {
        focusedPackId = pack.id;
        selectedSamplePath.clear();
    }

    void clearSelection()
    {
        selectedSamplePath.clear();
    }

    // View-specific pixel-size helpers. Adjust ranges later if needed.
    [[nodiscard]] int gridTileSize() const
    {
        constexpr int minSize = 80;
        constexpr int maxSize = 320;
        const auto t = std::clamp(thumbnailZoomGrid, 0.0, 1.0);
        return static_cast<int>(std::round(minSize + t * (maxSize - minSize)));
    }

    [[nodiscard]] int listThumbnailSize() const
    {
        constexpr int minSize = 48;
        constexpr int maxSize = 128;
        const auto t = std::clamp(thumbnailZoomList, 0.0, 1.0);
        return static_cast<int>(std::round(minSize + t * (maxSize - minSize)));
    }

    [[nodiscard]] int coverFlowCardSize() const
    {
        constexpr int minSize = 200;
        constexpr int maxSize = 360;
        const auto t = std::clamp(thumbnailZoomCoverFlow, 0.0, 1.0);
        return static_cast<int>(std::round(minSize + t * (maxSize - minSize)));
    }
};
}
