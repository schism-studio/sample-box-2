#pragma once

#include "LibrarySnapshot.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
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

// Shared, host-independent browser state. MainPanel and views read and write
// this model so standalone and VST3 use identical focus, selection, and zoom.
// Optimized with an O(1) index map to scale across thousands of packs.
struct BrowseState
{
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void browseSelectionChanged() {}
        virtual void browseSnapshotChanged() {}
        virtual void browseViewModeChanged() {}
    };

    BrowseLevel level = BrowseLevel::packs;
    PackViewMode packViewMode = PackViewMode::coverFlow;

    LibrarySnapshotPtr snapshot;
    std::string focusedPackId;
    std::filesystem::path selectedSamplePath;
    std::string searchQuery;
    std::vector<std::string> extensionFilter;

    // Continuous zoom per view mode, 0.0–1.0. Stored here so the user's zoom
    // choice survives view switches.
    double thumbnailZoomGrid = 0.5;
    double thumbnailZoomList = 0.5;
    double thumbnailZoomCoverFlow = 0.5;

    void addListener(Listener* listener)
    {
        if (listener != nullptr && std::find(listeners.begin(), listeners.end(), listener) == listeners.end())
            listeners.push_back(listener);
    }

    void removeListener(Listener* listener)
    {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }

    void setSnapshot(LibrarySnapshotPtr newSnapshot)
    {
        snapshot = std::move(newSnapshot);
        level = BrowseLevel::packs;
        rebuildIndexMap();

        // If previously focused pack no longer exists, clear selection
        if (!focusedPackId.empty() && packIdToIndex.find(focusedPackId) == packIdToIndex.end())
        {
            focusedPackId.clear();
            selectedSamplePath.clear();
        }

        notifySnapshotChanged();
        notifySelectionChanged();
    }

    [[nodiscard]] std::size_t getPackCount() const noexcept
    {
        return snapshot != nullptr ? snapshot->packs.size() : 0u;
    }

    [[nodiscard]] std::optional<std::size_t> findFocusedPackIndex() const
    {
        if (snapshot == nullptr || focusedPackId.empty())
            return std::nullopt;

        const auto it = packIdToIndex.find(focusedPackId);
        if (it != packIdToIndex.end() && it->second < snapshot->packs.size())
            return it->second;

        return std::nullopt;
    }

    [[nodiscard]] const SamplePack* findFocusedPack() const
    {
        const auto idx = findFocusedPackIndex();
        if (idx.has_value() && snapshot != nullptr)
            return &snapshot->packs[*idx];

        return nullptr;
    }

    [[nodiscard]] bool hasFocusedPack() const
    {
        return findFocusedPack() != nullptr;
    }

    void focusPackById(const std::string& packId)
    {
        if (focusedPackId == packId)
            return;

        focusedPackId = packId;
        selectedSamplePath.clear();
        notifySelectionChanged();
    }

    void focusPackByIndex(std::size_t index)
    {
        if (snapshot != nullptr && index < snapshot->packs.size())
            focusPackById(snapshot->packs[index].id);
    }

    void focusPack(const SamplePack& pack)
    {
        focusPackById(pack.id);
    }

    void selectSample(const std::filesystem::path& samplePath)
    {
        if (selectedSamplePath == samplePath)
            return;

        selectedSamplePath = samplePath;
        notifySelectionChanged();
    }

    void clearSelection()
    {
        if (focusedPackId.empty() && selectedSamplePath.empty())
            return;

        focusedPackId.clear();
        selectedSamplePath.clear();
        notifySelectionChanged();
    }

    void setPackViewMode(PackViewMode newMode)
    {
        if (packViewMode == newMode)
            return;

        packViewMode = newMode;
        for (auto* listener : listeners)
            if (listener != nullptr)
                listener->browseViewModeChanged();
    }

    // View-specific pixel-size helpers.
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

private:
    void rebuildIndexMap()
    {
        packIdToIndex.clear();
        if (snapshot == nullptr)
            return;

        packIdToIndex.reserve(snapshot->packs.size());
        for (std::size_t i = 0; i < snapshot->packs.size(); ++i)
        {
            packIdToIndex[snapshot->packs[i].id] = i;
        }
    }

    void notifySelectionChanged()
    {
        for (auto* listener : listeners)
            if (listener != nullptr)
                listener->browseSelectionChanged();
    }

    void notifySnapshotChanged()
    {
        for (auto* listener : listeners)
            if (listener != nullptr)
                listener->browseSnapshotChanged();
    }

    std::unordered_map<std::string, std::size_t> packIdToIndex;
    std::vector<Listener*> listeners;
};
}
