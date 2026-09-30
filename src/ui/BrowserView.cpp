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
    addAndMakeVisible(*carousel);
}

BrowserView::~BrowserView() = default;

void BrowserView::refresh()
{
    carousel->refresh();
}

void BrowserView::paint(juce::Graphics& graphics)
{
    graphics.fillAll(theme::background);
}

void BrowserView::resized()
{
    carousel->setBounds(getLocalBounds());
}
}
