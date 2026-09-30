#include "CoverArtCarousel.h"

#include "ArtworkCache.h"
#include "CoverArtCard.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace samplebox
{
namespace
{
constexpr float kScrollSettleEpsilon = 0.001f;
}

CoverArtCarousel::CoverArtCarousel(BrowseState& state, SampleSelected sampleSelectedCallback)
    : browseState(state),
      artworkCache(std::make_unique<ArtworkCache>()),
      animationClock([this](double deltaSeconds) { advanceAnimation(deltaSeconds); }),
      onSampleSelected(std::move(sampleSelectedCallback))
{
    browseState.addListener(this);

    // Allocate fixed pool of reusable cards once
    for (int i = 0; i < kMaxVisibleCards; ++i)
    {
        auto card = std::make_unique<CoverArtCard>(*artworkCache, [this](std::size_t clickedPackIndex) {
            auditionAndSelect(clickedPackIndex);
        });
        card->setVisible(false);
        addChildComponent(*card);
        cardPool.push_back(std::move(card));
    }
}

CoverArtCarousel::~CoverArtCarousel()
{
    browseState.removeListener(this);
}

void CoverArtCarousel::refresh()
{
    const auto totalPacks = browseState.getPackCount();
    if (totalPacks == 0)
    {
        targetScrollPosition = 0.0f;
        scrollPosition = 0.0f;
    }
    else
    {
        // If there's an active focused pack, make sure we align to it
        const auto focusedIdx = browseState.findFocusedPackIndex();
        if (focusedIdx.has_value())
        {
            targetScrollPosition = static_cast<float>(*focusedIdx);
            scrollPosition = targetScrollPosition;
        }
        else
        {
            targetScrollPosition = juce::jlimit(0.0f, static_cast<float>(totalPacks - 1), targetScrollPosition);
            scrollPosition = juce::jlimit(0.0f, static_cast<float>(totalPacks - 1), scrollPosition);
        }
    }

    layoutVisibleCards();
}

void CoverArtCarousel::scrollToPack(const std::string& packId)
{
    browseState.focusPackById(packId);
    const auto idx = browseState.findFocusedPackIndex();
    if (idx.has_value())
        scrollToPackIndex(*idx);
}

void CoverArtCarousel::scrollToPackIndex(std::size_t index)
{
    const auto totalPacks = browseState.getPackCount();
    if (totalPacks == 0)
        return;

    targetScrollPosition = juce::jlimit(0.0f, static_cast<float>(totalPacks - 1), static_cast<float>(index));
    startAnimationIfNeeded();
}

void CoverArtCarousel::browseSelectionChanged()
{
    layoutVisibleCards();
}

void CoverArtCarousel::browseSnapshotChanged()
{
    refresh();
}

void CoverArtCarousel::auditionAndSelect(std::size_t packIndex)
{
    const auto* snapshot = browseState.snapshot.get();
    if (snapshot == nullptr || packIndex >= snapshot->packs.size())
        return;

    // 1. Focus pack in state
    browseState.focusPackByIndex(packIndex);

    // 2. Animate target to center the clicked pack
    targetScrollPosition = static_cast<float>(packIndex);
    startAnimationIfNeeded();

    // 3. Play random sample
    const auto& pack = snapshot->packs[packIndex];
    if (!pack.sampleFiles.empty() && onSampleSelected)
    {
        std::uniform_int_distribution<std::size_t> chooseSample(0, pack.sampleFiles.size() - 1);
        const auto& chosenSample = pack.sampleFiles[chooseSample(randomEngine)];
        browseState.selectSample(chosenSample);
        onSampleSelected(chosenSample);
    }
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
        scrollPosition = targetScrollPosition;
        animationClock.stop();

        // Update focused pack index to the settled center card if not already focused
        const auto totalPacks = browseState.getPackCount();
        if (totalPacks > 0)
        {
            const auto centerIndex = static_cast<std::size_t>(juce::roundToInt(scrollPosition));
            if (centerIndex < totalPacks)
                browseState.focusPackByIndex(centerIndex);
        }

        layoutVisibleCards();
        return;
    }

    const auto smoothing = static_cast<float>(std::min(1.0, deltaSeconds * 12.0));
    scrollPosition += remaining * smoothing;
    layoutVisibleCards();
}

void CoverArtCarousel::resized()
{
    layoutVisibleCards();
}

void CoverArtCarousel::layoutVisibleCards()
{
    const auto totalPacks = browseState.getPackCount();
    if (totalPacks == 0 || browseState.snapshot == nullptr)
    {
        for (auto& card : cardPool)
            card->setVisible(false);
        return;
    }

    const auto centreX = getWidth() * 0.5f;
    const auto centreY = getHeight() * 0.5f;

    // Determine virtual index window
    const int minIndex = std::max(0, static_cast<int>(std::floor(scrollPosition - kVisibleReach)));
    const int maxIndex = std::min(static_cast<int>(totalPacks - 1), static_cast<int>(std::ceil(scrollPosition + kVisibleReach)));

    const auto focusedIndexOpt = browseState.findFocusedPackIndex();

    std::size_t poolIdx = 0;

    for (int index = minIndex; index <= maxIndex && poolIdx < cardPool.size(); ++index, ++poolIdx)
    {
        auto& card = *cardPool[poolIdx];
        card.bindToPack(browseState.snapshot, static_cast<std::size_t>(index));

        const auto offset = static_cast<float>(index) - scrollPosition;
        const auto distance = std::abs(offset);
        const auto scale = std::max(0.72f, 1.0f - distance * 0.12f);
        const auto opacity = std::max(0.28f, 1.0f - distance * 0.24f);
        const auto width = theme::carouselCardWidth * scale;
        const auto height = theme::carouselCardHeight * scale;
        const auto x = centreX + offset * kCardSpacing - width * 0.5f;
        const auto y = centreY - height * 0.5f + distance * 20.0f;

        const bool isFocused = focusedIndexOpt.has_value() && (*focusedIndexOpt == static_cast<std::size_t>(index));
        const bool isVisuallyCentered = distance < 0.5f;

        card.setVisible(true);
        card.setBounds(juce::roundToInt(x), juce::roundToInt(y), juce::roundToInt(width), juce::roundToInt(height));
        card.setVisualState(scale, opacity, isFocused || isVisuallyCentered);

        // Z-order: bring closer cards forward
        card.toFront(false);
    }

    // Hide any unused cards in the pool
    for (; poolIdx < cardPool.size(); ++poolIdx)
    {
        cardPool[poolIdx]->setVisible(false);
    }
}

void CoverArtCarousel::mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& details)
{
    const auto totalPacks = browseState.getPackCount();
    if (totalPacks == 0)
        return;

    const auto movement = details.deltaY != 0.0f ? details.deltaY : details.deltaX;
    targetScrollPosition = juce::jlimit(0.0f, static_cast<float>(totalPacks - 1), targetScrollPosition + movement * 2.0f);

    startAnimationIfNeeded();
}
}
