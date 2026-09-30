#pragma once

#include "../core/BrowseState.h"
#include "AnimationClock.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <random>
#include <vector>

namespace samplebox
{
class ArtworkCache;
class CoverArtCard;

// Virtualized Cover Art Carousel.
// Recycles a fixed pool of ~15 visible CoverArtCard components, scaling smoothly
// to thousands of packs with near-zero idle CPU and bounded memory overhead.
class CoverArtCarousel final : public juce::Component,
                               private BrowseState::Listener
{
public:
    using SampleSelected = std::function<void(const std::filesystem::path&)>;

    CoverArtCarousel(BrowseState& state, SampleSelected onSampleSelected = {});
    ~CoverArtCarousel() override;

    void refresh();
    void scrollToPack(const std::string& packId);
    void scrollToPackIndex(std::size_t index);

    void resized() override;
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& details) override;

private:
    // BrowseState::Listener
    void browseSelectionChanged() override;
    void browseSnapshotChanged() override;

    void advanceAnimation(double deltaSeconds);
    void layoutVisibleCards();
    void auditionAndSelect(std::size_t packIndex);
    void startAnimationIfNeeded();

    BrowseState& browseState;
    std::unique_ptr<ArtworkCache> artworkCache;
    std::vector<std::unique_ptr<CoverArtCard>> cardPool;
    AnimationClock animationClock;
    SampleSelected onSampleSelected;
    std::mt19937 randomEngine { std::random_device{}() };

    float scrollPosition = 0.0f;
    float targetScrollPosition = 0.0f;

    static constexpr int kMaxVisibleCards = 15;
    static constexpr float kCardSpacing = 190.0f;
    static constexpr float kVisibleReach = 7.0f;
};
}
