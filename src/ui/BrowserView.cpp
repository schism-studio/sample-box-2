#include "BrowserView.h"

#include "CoverArtCarousel.h"
#include "Theme.h"

#include <utility>

namespace samplebox
{
BrowserView::BrowserView(SampleSelected onSampleSelected)
    : carousel(std::make_unique<CoverArtCarousel>(std::move(onSampleSelected)))
{
    addAndMakeVisible(*carousel);
}

BrowserView::~BrowserView() = default;

void BrowserView::setLibrary(LibrarySnapshotPtr snapshot)
{
    carousel->setLibrary(std::move(snapshot));
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
