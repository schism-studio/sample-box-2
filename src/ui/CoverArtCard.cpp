#include "CoverArtCard.h"

#include "ArtworkCache.h"
#include "Theme.h"

#include <cmath>
#include <utility>

namespace samplebox
{
CoverArtCard::CoverArtCard(ArtworkCache& cache,
                           std::function<void(std::size_t)> clickCallback)
    : artworkCache(cache),
      onPackClicked(std::move(clickCallback))
{
}

void CoverArtCard::bindToPack(LibrarySnapshotPtr snapshot, std::size_t indexOfPack)
{
    const bool packChanged = (librarySnapshot != snapshot) || (packIndex != indexOfPack);
    librarySnapshot = std::move(snapshot);
    packIndex = indexOfPack;

    if (packChanged)
        repaint();
}

void CoverArtCard::mouseDown(const juce::MouseEvent&)
{
    if (onPackClicked && hasValidPack())
        onPackClicked(packIndex);
}

void CoverArtCard::setVisualState(float newScale, float newOpacity, bool isSelected)
{
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
    const auto* pack = getPack();
    if (pack == nullptr)
        return;

    const auto bounds = getLocalBounds().toFloat();
    graphics.setOpacity(opacity);
    graphics.setColour(theme::surface);
    graphics.fillRoundedRectangle(bounds, theme::cardCornerRadius);

    const auto artworkBounds = bounds.reduced(12.0f).withTrimmedBottom(46.0f);
    constexpr int coverFlowThumbnailSize = 320;
    const auto thumbnailWidth = coverFlowThumbnailSize;
    const auto thumbnailHeight = coverFlowThumbnailSize;

    const juce::Component::SafePointer<CoverArtCard> safeThis(this);
    const auto capturedIndex = packIndex;

    const auto image = artworkCache.getThumbnail(
        pack->coverArtPath,
        thumbnailWidth,
        thumbnailHeight,
        [safeThis, capturedIndex]()
        {
            // Verify component is still bound to the same pack
            if (safeThis != nullptr && safeThis->packIndex == capturedIndex)
                safeThis->repaint();
        });

    graphics.drawImageWithin(image,
                             artworkBounds.getX(),
                             artworkBounds.getY(),
                             artworkBounds.getWidth(),
                             artworkBounds.getHeight(),
                             juce::RectanglePlacement::centred);

    graphics.setColour(theme::textPrimary);
    graphics.setFont(selected ? 17.0f : 15.0f);
    graphics.drawFittedText(pack->title,
                            getLocalBounds().reduced(12).removeFromBottom(34),
                            juce::Justification::centred,
                            2);
}
}
