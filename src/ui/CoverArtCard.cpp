#include "CoverArtCard.h"

#include "ArtworkCache.h"
#include "Theme.h"

#include <cmath>
#include <utility>

namespace samplebox
{
CoverArtCard::CoverArtCard(LibrarySnapshotPtr snapshot,
                           std::size_t indexOfPack,
                           ArtworkCache& cache,
                           std::function<void(std::size_t)> clickCallback)
    : librarySnapshot(std::move(snapshot)),
      packIndex(indexOfPack),
      artworkCache(cache),
      onPackClicked(std::move(clickCallback))
{
    jassert(librarySnapshot != nullptr);
    jassert(packIndex < librarySnapshot->packs.size());
}

void CoverArtCard::mouseDown(const juce::MouseEvent&)
{
    if (onPackClicked)
        onPackClicked(packIndex);
}

void CoverArtCard::setVisualState(float newScale, float newOpacity, bool isSelected)
{
    // The carousel calls this for every card from resized(), which its
    // animation tick drives once per frame. Repainting unconditionally meant
    // every card was invalidated 60 times a second even when nothing about it
    // had changed, so only repaint on an actual visual difference. The
    // thresholds are well below one pixel of scale at any plausible card size.
    const auto changed = std::abs(newScale - scale) > 0.001f
                      || std::abs(newOpacity - opacity) > 0.001f
                      || isSelected != selected;

    scale = newScale;
    opacity = newOpacity;
    selected = isSelected;

    if (changed)
        repaint();
}

void CoverArtCard::paint(juce::Graphics& graphics)
{
    const auto& pack = getPack();

    const auto bounds = getLocalBounds().toFloat();
    graphics.setOpacity(opacity);
    graphics.setColour(theme::surface);
    graphics.fillRoundedRectangle(bounds, theme::cardCornerRadius);

    const auto artworkBounds = bounds.reduced(12.0f).withTrimmedBottom(46.0f);
    const auto image = artworkCache.imageFor(pack.coverArtPath);

    if (image.isValid())
        graphics.drawImageWithin(image, artworkBounds.getX(), artworkBounds.getY(), artworkBounds.getWidth(), artworkBounds.getHeight(), juce::RectanglePlacement::centred);
    else
    {
        graphics.setColour(theme::accent.withAlpha(0.25f));
        graphics.fillRoundedRectangle(artworkBounds, theme::cardCornerRadius - 4.0f);
    }

    graphics.setColour(theme::textPrimary);
    graphics.setFont(selected ? 17.0f : 15.0f);
    graphics.drawFittedText(pack.title, getLocalBounds().reduced(12).removeFromBottom(34), juce::Justification::centred, 2);
}
}
