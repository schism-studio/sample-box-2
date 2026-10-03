#pragma once

#include "../core/BrowseState.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <filesystem>
#include <functional>
#include <memory>

namespace samplebox
{
class CoverArtCarousel;

class BrowserView final : public juce::Component,
                          private BrowseState::Listener
{
public:
    using SampleSelected = std::function<void(const std::filesystem::path&)>;

    BrowserView(BrowseState& browseState, SampleSelected onSampleSelected = {});
    ~BrowserView() override;

    void refresh();

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void browseViewModeChanged() override;
    void updateActiveView();

    BrowseState& state;
    std::unique_ptr<CoverArtCarousel> carousel;
};
}
