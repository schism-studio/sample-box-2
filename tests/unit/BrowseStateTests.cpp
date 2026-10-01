#include "../../src/core/BrowseState.h"

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

struct TestListener : public samplebox::BrowseState::Listener
{
    int selectionChangedCount = 0;
    int snapshotChangedCount = 0;
    int viewModeChangedCount = 0;

    void browseSelectionChanged() override { ++selectionChangedCount; }
    void browseSnapshotChanged() override { ++snapshotChangedCount; }
    void browseViewModeChanged() override { ++viewModeChangedCount; }
};

samplebox::LibrarySnapshotPtr createMockLibrary(std::size_t count)
{
    auto snapshot = std::make_shared<samplebox::LibrarySnapshot>();

    snapshot->packs.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
    {
        samplebox::SamplePack pack;
        pack.id = "pack-" + std::to_string(i);
        pack.title = "Pack Title " + std::to_string(i);
        pack.sampleFiles = {
            "D:/MockLibrary/pack" + std::to_string(i) + "/Kick.wav",
            "D:/MockLibrary/pack" + std::to_string(i) + "/Snare.wav"
        };
        snapshot->packs.push_back(std::move(pack));
    }
    return snapshot;
}

void testO1LookupsAndSelection()
{
    constexpr std::size_t kPackCount = 2000;
    auto mockLib = createMockLibrary(kPackCount);

    samplebox::BrowseState state;
    TestListener listener;
    state.addListener(&listener);

    state.setSnapshot(mockLib);
    CHECK_EQ(state.getPackCount(), kPackCount);
    CHECK_EQ(listener.snapshotChangedCount, 1);
    CHECK_EQ(listener.selectionChangedCount, 1);

    // Focus by index
    state.focusPackByIndex(450);
    CHECK_EQ(state.focusedPackId, std::string("pack-450"));
    CHECK(state.findFocusedPackIndex().has_value());
    CHECK_EQ(*state.findFocusedPackIndex(), static_cast<std::size_t>(450));
    CHECK(state.findFocusedPack() != nullptr);
    CHECK_EQ(state.findFocusedPack()->title, std::string("Pack Title 450"));
    CHECK_EQ(listener.selectionChangedCount, 2);

    // Focus by ID
    state.focusPackById("pack-1999");
    CHECK(state.findFocusedPackIndex().has_value());
    CHECK_EQ(*state.findFocusedPackIndex(), static_cast<std::size_t>(1999));
    CHECK_EQ(listener.selectionChangedCount, 3);

    // Select sample
    state.selectSample("D:/MockLibrary/pack1999/Kick.wav");
    CHECK_EQ(state.selectedSamplePath, std::filesystem::path("D:/MockLibrary/pack1999/Kick.wav"));
    CHECK_EQ(listener.selectionChangedCount, 4);

    // Clear selection
    state.clearSelection();
    CHECK_EQ(state.focusedPackId, std::string{});
    CHECK_EQ(state.selectedSamplePath, std::filesystem::path{});
    CHECK(!state.findFocusedPackIndex().has_value());
    CHECK_EQ(listener.selectionChangedCount, 5);

    // View mode change
    state.setPackViewMode(samplebox::PackViewMode::grid);
    CHECK(state.packViewMode == samplebox::PackViewMode::grid);
    CHECK_EQ(listener.viewModeChangedCount, 1);

    state.removeListener(&listener);
}

void testSnapshotReplacementClearsMissingFocus()
{
    auto lib1 = createMockLibrary(10);
    samplebox::BrowseState state;
    state.setSnapshot(lib1);
    state.focusPackById("pack-8");
    CHECK_EQ(state.focusedPackId, std::string("pack-8"));

    // Replace with smaller library missing pack-8
    auto lib2 = createMockLibrary(5);
    state.setSnapshot(lib2);
    CHECK_EQ(state.focusedPackId, std::string{});
    CHECK(!state.findFocusedPackIndex().has_value());
}
}

int main()
{
    testO1LookupsAndSelection();
    testSnapshotReplacementClearsMissingFocus();

    std::printf("BrowseStateTests: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
