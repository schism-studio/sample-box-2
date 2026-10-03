#include "BrowserView.h"

#include "CoverArtCarousel.h"
#include "Theme.h"

#include <utility>

namespace samplebox
{
BrowserView::BrowserView(BrowseState& browseState, SampleSelected onSampleSelected)
    : state(browseState),
      carousel(std::make_unique<CoverArtCarousel>(browseState, std::move(onSampleSelected)))
{
    state.addListener(this);
    addAndMakeVisible(*carousel);
    updateActiveView();
}

BrowserView::~BrowserView()
{
    state.removeListener(this);
}

void BrowserView::refresh()
{
    carousel->refresh();
    updateActiveView();
}

void BrowserView::paint(juce::Graphics& graphics)
{
    graphics.fillAll(theme::background);

    if (state.packViewMode == PackViewMode::coverFlow)
        return;

    const auto modeName = state.packViewMode == PackViewMode::grid ? "Grid" : "List";
    const auto bounds = getLocalBounds().toFloat();

    graphics.setColour(theme::textPrimary);
    graphics.setFont(20.0f);
    graphics.drawText(juce::String(modeName) + " view is under construction",
                      bounds.withTrimmedBottom(22.0f),
                      juce::Justification::centred);

    graphics.setColour(theme::textSecondary);
    graphics.setFont(14.0f);
    graphics.drawText("Use Carousel in Options to browse packs.",
                      bounds.withTrimmedTop(22.0f),
                      juce::Justification::centred);
}

void BrowserView::resized()
{
    carousel->setBounds(getLocalBounds());
}

void BrowserView::browseViewModeChanged()
{
    updateActiveView();
}

void BrowserView::updateActiveView()
{
    const bool showCarousel = state.packViewMode == PackViewMode::coverFlow;
    carousel->setVisible(showCarousel);
    repaint();
}
}
