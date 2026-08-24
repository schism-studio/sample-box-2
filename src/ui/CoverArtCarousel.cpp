#include "CoverArtCarousel.h"

#include "ArtworkCache.h"
#include "CoverArtCard.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>

namespace samplebox
{
namespace
{
// Below this much remaining travel the motion is sub-pixel at any plausible
// cover size, so the carousel is treated as settled and the clock is stopped.
constexpr float kScrollSettleEpsilon = 0.001f;
}

CoverArtCarousel::CoverArtCarousel()
    : artworkCache(std::make_unique<ArtworkCache>()),
      animationClock([this](double deltaSeconds) { advanceAnimation(deltaSeconds); })
{
    // The clock is deliberately not started here. It runs only while the
    // carousel is actually moving, and stops itself once it settles.
}

CoverArtCarousel::~CoverArtCarousel() = default;

void CoverArtCarousel::setLibrary(LibrarySnapshotPtr snapshot)
{
    // Destroy the cards that point into the previous snapshot *before* dropping
    // this carousel's reference to it. Order matters less now that each card
    // holds its own share of the snapshot, but tearing down in this order keeps
    // the lifetime obvious rather than relying on the shared count.
    cards.clear();

    library = std::move(snapshot);
    targetScrollPosition = 0.0f;
    scrollPosition = 0.0f;
    rebuildCards();
}

void CoverArtCarousel::rebuildCards()
{
    cards.clear();

    if (library == nullptr)
    {
        resized();
        return;
    }

    for (std::size_t index = 0; index < library->packs.size(); ++index)
    {
        auto card = std::make_unique<CoverArtCard>(library, index, *artworkCache);
        addAndMakeVisible(*card);
        cards.push_back(std::move(card));
    }

    resized();
}

void CoverArtCarousel::startAnimationIfNeeded()
{
    if (std::abs(targetScrollPosition - scrollPosition) > kScrollSettleEpsilon)
        animationClock.start();
}

void CoverArtCarousel::advanceAnimation(double deltaSeconds)
{
    const auto remaining = targetScrollPosition - scrollPosition;

    if (std::abs(remaining) <= kScrollSettleEpsilon)
    {
        // Land exactly on the target instead of asymptotically approaching it,
        // then stop the clock. Without this the exponential ease never quite
        // arrives and the timer would run forever.
        scrollPosition = targetScrollPosition;
        animationClock.stop();
        resized();
        return;
    }

    const auto smoothing = static_cast<float>(std::min(1.0, deltaSeconds * 12.0));
    scrollPosition += remaining * smoothing;
    resized();
}

void CoverArtCarousel::resized()
{
    const auto centreX = getWidth() * 0.5f;
    const auto centreY = getHeight() * 0.5f;

    for (size_t index = 0; index < cards.size(); ++index)
    {
        const auto offset = static_cast<float>(index) - scrollPosition;
        const auto distance = std::abs(offset);
        const auto scale = std::max(0.72f, 1.0f - distance * 0.12f);
        const auto opacity = std::max(0.28f, 1.0f - distance * 0.24f);
        const auto width = theme::carouselCardWidth * scale;
        const auto height = theme::carouselCardHeight * scale;
        const auto x = centreX + offset * 190.0f - width * 0.5f;
        const auto y = centreY - height * 0.5f + distance * 20.0f;

        cards[index]->setBounds(juce::roundToInt(x), juce::roundToInt(y), juce::roundToInt(width), juce::roundToInt(height));
        cards[index]->setVisualState(scale, opacity, distance < 0.5f);
    }
}

void CoverArtCarousel::mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& details)
{
    if (cards.empty())
        return;

    const auto movement = details.deltaY != 0.0f ? -details.deltaY : details.deltaX;
    targetScrollPosition = juce::jlimit(0.0f, static_cast<float>(cards.size() - 1), targetScrollPosition + movement * 2.0f);

    startAnimationIfNeeded();
}
}
