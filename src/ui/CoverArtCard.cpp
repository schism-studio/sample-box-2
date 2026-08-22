#include "CoverArtCard.h"

#include "ArtworkCache.h"
#include "Theme.h"

namespace samplebox
{
CoverArtCard::CoverArtCard(const SamplePack& packToDisplay, ArtworkCache& cache)
    : pack(packToDisplay), artworkCache(cache)
{
}

void CoverArtCard::setVisualState(float newScale, float newOpacity, bool isSelected)
{
    scale = newScale;
    opacity = newOpacity;
    selected = isSelected;
    repaint();
}

void CoverArtCard::paint(juce::Graphics& graphics)
{
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
