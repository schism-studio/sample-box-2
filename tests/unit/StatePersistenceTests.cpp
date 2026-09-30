#include "../../src/core/StatePersistence.h"

#include <cstdio>
#include <string>

namespace
{
int failures = 0;
int checks = 0;

void reportCheck(bool passed, const char* expression, const char* file, int line)
{
    ++checks;
    if (!passed)
    {
        ++failures;
        std::printf("FAIL %s:%d\n  %s\n", file, line, expression);
    }
}

template <typename A, typename B>
void reportEqual(const A& actual, const B& expected, const char* expression,
                 const char* file, int line)
{
    ++checks;
    if (!(actual == static_cast<A>(expected)))
    {
        ++failures;
        std::printf("FAIL %s:%d\n  %s\n", file, line, expression);
    }
}

#define CHECK(expr) reportCheck((expr), #expr, __FILE__, __LINE__)
#define CHECK_EQ(actual, expected) \
    reportEqual((actual), (expected), #actual " == " #expected, __FILE__, __LINE__)

void testRoundTripState()
{
    samplebox::StatePersistence::InstanceState original;
    original.sampleLibraryPath = "D:\\Samples\\MyPacks";
    original.packViewMode = samplebox::PackViewMode::grid;
    original.focusedPackId = "pack-123-abc";
    original.selectedSamplePath = "D:\\Samples\\MyPacks\\Kick.wav";
    original.thumbnailZoomGrid = 0.75;
    original.thumbnailZoomList = 0.25;
    original.thumbnailZoomCoverFlow = 0.85;
    original.randomDragSampleCount = 8;
    original.showCoverTitles = true;

    auto xml = samplebox::StatePersistence::createXmlFromState(original);
    CHECK(xml != nullptr);

    const auto restored = samplebox::StatePersistence::restoreStateFromXml(*xml);
    CHECK_EQ(restored.sampleLibraryPath, original.sampleLibraryPath);
    CHECK(restored.packViewMode == original.packViewMode);
    CHECK_EQ(restored.focusedPackId, original.focusedPackId);
    CHECK_EQ(restored.selectedSamplePath, original.selectedSamplePath);
    CHECK_EQ(restored.thumbnailZoomGrid, original.thumbnailZoomGrid);
    CHECK_EQ(restored.thumbnailZoomList, original.thumbnailZoomList);
    CHECK_EQ(restored.thumbnailZoomCoverFlow, original.thumbnailZoomCoverFlow);
    CHECK_EQ(restored.randomDragSampleCount, original.randomDragSampleCount);
    CHECK_EQ(restored.showCoverTitles, original.showCoverTitles);
}

void testDefaultStateFallback()
{
    juce::XmlElement emptyXml("OTHER_TAG");
    const auto fallback = samplebox::StatePersistence::restoreStateFromXml(emptyXml);
    CHECK_EQ(fallback.sampleLibraryPath, juce::String{});
    CHECK(fallback.packViewMode == samplebox::PackViewMode::coverFlow);
    CHECK_EQ(fallback.randomDragSampleCount, 5);
    CHECK_EQ(fallback.showCoverTitles, false);
}
}

int main()
{
    testRoundTripState();
    testDefaultStateFallback();

    std::printf("StatePersistenceTests: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
