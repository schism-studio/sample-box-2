#include "BrowserView.h"

#include "CoverArtCarousel.h"
#include "Theme.h"

namespace samplebox
{
BrowserView::BrowserView() : carousel(std::make_unique<CoverArtCarousel>())
{
    addAndMakeVisible(*carousel);
}

void BrowserView::setLibrary(LibrarySnapshot snapshot)
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
