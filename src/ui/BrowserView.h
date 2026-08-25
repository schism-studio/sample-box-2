#pragma once

#include "../core/LibrarySnapshot.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <filesystem>
#include <functional>
#include <memory>

namespace samplebox
{
class CoverArtCarousel;

class BrowserView final : public juce::Component
{
public:
    using SampleSelected = std::function<void(const std::filesystem::path&)>;

    explicit BrowserView(SampleSelected onSampleSelected = {});
    ~BrowserView() override;

    void setLibrary(LibrarySnapshotPtr snapshot);
    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    std::unique_ptr<CoverArtCarousel> carousel;
};
}
