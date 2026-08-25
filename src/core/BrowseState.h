#pragma once

#include "LibrarySnapshot.h"

#include <algorithm>
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
};
}
