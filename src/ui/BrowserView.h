#pragma once

#include "../core/LibrarySnapshot.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace samplebox
{
class CoverArtCarousel;

class BrowserView final : public juce::Component
{
public:
    BrowserView();
    ~BrowserView() override;

    void setLibrary(LibrarySnapshot snapshot);
    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    std::unique_ptr<CoverArtCarousel> carousel;
};
}
