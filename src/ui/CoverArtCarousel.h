#pragma once

#include "../core/LibrarySnapshot.h"
#include "AnimationClock.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <vector>

namespace samplebox
{
class ArtworkCache;
class CoverArtCard;

class CoverArtCarousel final : public juce::Component
{
public:
    CoverArtCarousel();
    ~CoverArtCarousel();

    void setLibrary(LibrarySnapshotPtr snapshot);
    void resized() override;
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& details) override;

private:
    void rebuildCards();
    void advanceAnimation(double deltaSeconds);

    // Starts the animation clock only if the carousel is actually away from
    // where it wants to be. Cheap to call on every input event.
    void startAnimationIfNeeded();

    LibrarySnapshotPtr library;
    std::unique_ptr<ArtworkCache> artworkCache;
    std::vector<std::unique_ptr<CoverArtCard>> cards;
    AnimationClock animationClock;
    float scrollPosition = 0.0f;
    float targetScrollPosition = 0.0f;
};
}
