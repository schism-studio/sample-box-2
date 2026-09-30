#include "BrowserView.h"

#include "CoverArtCarousel.h"
#include "Theme.h"

#include <utility>

namespace samplebox
{
BrowserView::BrowserView(BrowseState& browseState, SampleSelected onSampleSelected)
    : state(browseState),
      carousel(std::make_unique<CoverArtCarousel>(std::move(onSampleSelected)))
{
    addAndMakeVisible(*carousel);
}

BrowserView::~BrowserView() = default;

void BrowserView::refresh()
{
    // Cover Flow remains the only implemented renderer for this first state
    // commit. List and Grid will later render from this same BrowseState.
    carousel->setLibrary(state.snapshot);
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
