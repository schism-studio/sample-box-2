#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

#include <filesystem>
#include <functional>
#include <map>
#include <set>
#include <vector>

namespace samplebox
{
// Bounded async artwork cache. Image decoding is performed on one background
// worker. A key may have only one job in flight; callers receive a placeholder
// immediately and are notified on the message thread when that exact key is
// ready to repaint.
class ArtworkCache final
{
public:
    using ReadyCallback = std::function<void()>;

    explicit ArtworkCache(int maxEntries = 256);
    ~ArtworkCache();

    // Returns a cached thumbnail immediately when available. Otherwise returns
    // a placeholder, starts one load for this key if needed, and runs onReady
    // on the message thread when that thumbnail has been inserted.
    [[nodiscard]] juce::Image getThumbnail(const std::filesystem::path& path,
                                           int width,
                                           int height,
                                           ReadyCallback onReady = {});

    void clear();

private:
    struct CacheKey
    {
        std::filesystem::path path;
        int width = 0;
        int height = 0;

        bool operator<(const CacheKey& other) const
        {
            if (path < other.path) return true;
            if (path > other.path) return false;
            if (width < other.width) return true;
            if (width > other.width) return false;
            return height < other.height;
        }
    };

    struct CacheEntry
    {
        juce::Image image;
        std::uint64_t lastUsedTick = 0;
    };

    void startLoadIfNeeded(const CacheKey& key);
    void finishLoad(CacheKey key, juce::Image thumbnail);
    void evictLeastRecentlyUsed();
    [[nodiscard]] juce::Image createPlaceholder(int width, int height) const;

    juce::ThreadPool thumbnailLoader;
    std::map<CacheKey, CacheEntry> cache;
    std::set<CacheKey> pending;
    std::map<CacheKey, std::vector<ReadyCallback>> waitingCallbacks;
    juce::CriticalSection cacheMutex;

    std::uint64_t accessTick = 0;
    int maxEntries = 256;
    juce::Image placeholder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArtworkCache)
};
}
