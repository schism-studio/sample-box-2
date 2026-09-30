#include "ArtworkCache.h"

#include <utility>

namespace samplebox
{
ArtworkCache::ArtworkCache(int maxEntries)
    : thumbnailLoader(1),
      maxEntries(juce::jmax(1, maxEntries))
{
    placeholder = createPlaceholder(256, 256);
}

ArtworkCache::~ArtworkCache()
{
    thumbnailLoader.removeAllJobs(true, 5000);
}

juce::Image ArtworkCache::getThumbnail(const std::filesystem::path& path,
                                       int width,
                                       int height,
                                       ReadyCallback onReady)
{
    if (path.empty() || width <= 0 || height <= 0)
        return placeholder;

    const CacheKey key { path, width, height };

    {
        const juce::ScopedLock lock(cacheMutex);

        if (auto found = cache.find(key); found != cache.end())
        {
            found->second.lastUsedTick = ++accessTick;
            return found->second.image;
        }

        if (onReady)
            waitingCallbacks[key].push_back(std::move(onReady));
    }

    startLoadIfNeeded(key);
    return placeholder;
}

void ArtworkCache::clear()
{
    const juce::ScopedLock lock(cacheMutex);
    cache.clear();
    pending.clear();
    waitingCallbacks.clear();
}

void ArtworkCache::startLoadIfNeeded(const CacheKey& key)
{
    {
        const juce::ScopedLock lock(cacheMutex);

        if (cache.contains(key) || pending.contains(key))
            return;

        pending.insert(key);
    }

    thumbnailLoader.addJob(
        [this, key]()
        {
            auto decoded = juce::ImageFileFormat::loadFrom(
                juce::File(key.path.string()));

            juce::Image thumbnail;

            if (decoded.isValid())
            {
                thumbnail = juce::Image(juce::Image::ARGB,
                                        key.width,
                                        key.height,
                                        true);

                juce::Graphics graphics(thumbnail);
                graphics.setImageResamplingQuality(
                    juce::Graphics::highResamplingQuality);

                graphics.drawImageWithin(decoded,
                                         0,
                                         0,
                                         key.width,
                                         key.height,
                                         juce::RectanglePlacement::centred);
            }

            juce::MessageManager::callAsync(
                [this, key, thumbnail = std::move(thumbnail)]() mutable
                {
                    finishLoad(key, std::move(thumbnail));
                });
        });
}

void ArtworkCache::finishLoad(CacheKey key, juce::Image thumbnail)
{
    std::vector<ReadyCallback> callbacks;

    {
        const juce::ScopedLock lock(cacheMutex);

        pending.erase(key);

        if (thumbnail.isValid())
        {
            while (static_cast<int>(cache.size()) >= maxEntries)
                evictLeastRecentlyUsed();

            cache.insert_or_assign(
                key,
                CacheEntry { std::move(thumbnail), ++accessTick });
        }

        if (const auto waiting = waitingCallbacks.find(key);
            waiting != waitingCallbacks.end())
        {
            callbacks = std::move(waiting->second);
            waitingCallbacks.erase(waiting);
        }
    }

    // Completion is already on the JUCE message thread. Run callbacks outside
    // the lock because repaint may cause another cache request.
    for (auto& callback : callbacks)
    {
        if (callback)
            callback();
    }
}

void ArtworkCache::evictLeastRecentlyUsed()
{
    if (cache.empty())
        return;

    auto oldest = cache.begin();

    for (auto it = cache.begin(); it != cache.end(); ++it)
    {
        if (it->second.lastUsedTick < oldest->second.lastUsedTick)
            oldest = it;
    }

    cache.erase(oldest);
}

juce::Image ArtworkCache::createPlaceholder(int width, int height) const
{
    juce::Image image(juce::Image::ARGB,
                      juce::jmax(1, width),
                      juce::jmax(1, height),
                      true);

    juce::Graphics graphics(image);
    graphics.fillAll(juce::Colour(0xff1b1d22));
    graphics.setColour(juce::Colour(0x408b5cf6));
    graphics.drawRoundedRectangle(image.getBounds().toFloat().reduced(1.0f),
                                  6.0f,
                                  2.0f);
    return image;
}
}
