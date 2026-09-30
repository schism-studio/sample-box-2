// Unit tests for LibraryScanner's filtering, traversal and error recovery.
//
// These replace a SmokeTests.cpp whose entire body was `return 0`, which meant
// ctest reported a passing test suite while asserting nothing at all.
//
// No test framework: the project has no test dependency and adding one to
// assert on a handful of predicates is not worth the build cost. The harness
// below is about twenty lines and reports every failure with a file and line.

#include "../../src/indexing/LibraryScanner.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

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
        std::printf("FAIL %s:%d\n  %s\n  expected %s, got %s\n", file, line, expression,
                    std::to_string(expected).c_str(), std::to_string(actual).c_str());
    }
}

#define CHECK(expr) reportCheck((expr), #expr, __FILE__, __LINE__)
#define CHECK_EQ(actual, expected) \
    reportEqual((actual), (expected), #actual " == " #expected, __FILE__, __LINE__)

// A temp directory that cleans itself up, so a failing test cannot leave
// fixture trees behind for the next run to trip over.
class ScopedTempDirectory
{
public:
    explicit ScopedTempDirectory(const std::string& name)
    {
        std::error_code ec;
        root = std::filesystem::temp_directory_path(ec) / ("samplebox-test-" + name);
        std::filesystem::remove_all(root, ec);
        std::filesystem::create_directories(root, ec);
    }

    ~ScopedTempDirectory()
    {
        std::error_code ec;
        std::filesystem::remove_all(root, ec);
    }

    ScopedTempDirectory(const ScopedTempDirectory&) = delete;
    ScopedTempDirectory& operator=(const ScopedTempDirectory&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const { return root; }

    void makeDirectory(const std::string& relative) const
    {
        std::error_code ec;
        std::filesystem::create_directories(root / relative, ec);
    }

    // Writes a file with some non-zero content, since zero-byte files are
    // deliberately filtered and would otherwise make tests pass for the wrong
    // reason.
    void makeFile(const std::string& relative, const std::string& contents = "not-really-audio") const
    {
        const auto full = root / relative;
        std::error_code ec;
        std::filesystem::create_directories(full.parent_path(), ec);

        std::ofstream out(full, std::ios::binary | std::ios::trunc);
        out.write(contents.data(), (std::streamsize) contents.size());
    }

private:
    std::filesystem::path root;
};

const samplebox::SamplePack* findPack(const samplebox::LibrarySnapshot& snapshot,
                                      const std::string& title)
{
    for (const auto& pack : snapshot.packs)
        if (pack.title == title)
            return &pack;

    return nullptr;
}

//==============================================================================
void testExtensionPredicate()
{
    using samplebox::LibraryScanner;

    CHECK(LibraryScanner::hasSupportedAudioExtension("kick.wav"));
    CHECK(LibraryScanner::hasSupportedAudioExtension("kick.aif"));
    CHECK(LibraryScanner::hasSupportedAudioExtension("kick.aiff"));
    CHECK(LibraryScanner::hasSupportedAudioExtension("kick.mp3"));
    CHECK(LibraryScanner::hasSupportedAudioExtension("kick.flac"));

    // Case-insensitive: packs are full of .WAV from older tooling.
    CHECK(LibraryScanner::hasSupportedAudioExtension("KICK.WAV"));
    CHECK(LibraryScanner::hasSupportedAudioExtension("Kick.Wav"));

    CHECK(!LibraryScanner::hasSupportedAudioExtension("cover.png"));
    CHECK(!LibraryScanner::hasSupportedAudioExtension("readme.txt"));
    CHECK(!LibraryScanner::hasSupportedAudioExtension("noextension"));

    // ".wav" as a whole filename is a dotfile, not a sample. The extension
    // predicate is only asked about the extension, so it is isIgnoredFilename's
    // job to reject this - checked below.
    CHECK(LibraryScanner::isIgnoredFilename(".wav"));
}

void testIgnoredFilenamePredicate()
{
    using samplebox::LibraryScanner;

    // AppleDouble sidecars. The whole reason this predicate exists.
    CHECK(LibraryScanner::isIgnoredFilename("._kick.wav"));
    CHECK(LibraryScanner::isIgnoredFilename("._"));

    CHECK(LibraryScanner::isIgnoredFilename(".DS_Store"));
    CHECK(LibraryScanner::isIgnoredFilename(".hidden.wav"));

    CHECK(!LibraryScanner::isIgnoredFilename("kick.wav"));

    // An underscore prefix is a normal naming convention and must survive.
    CHECK(!LibraryScanner::isIgnoredFilename("_kick.wav"));

    // A dot elsewhere in the name is fine - version numbers are common.
    CHECK(!LibraryScanner::isIgnoredFilename("kick.v2.wav"));
}

void testIgnoredDirectoryPredicate()
{
    using samplebox::LibraryScanner;

    CHECK(LibraryScanner::isIgnoredDirectoryName("__MACOSX"));
    CHECK(LibraryScanner::isIgnoredDirectoryName("__macosx"));
    CHECK(LibraryScanner::isIgnoredDirectoryName("$RECYCLE.BIN"));
    CHECK(LibraryScanner::isIgnoredDirectoryName(".git"));

    CHECK(!LibraryScanner::isIgnoredDirectoryName("Kicks"));
    CHECK(!LibraryScanner::isIgnoredDirectoryName("808s & Snares"));

    // Not a substring match: a pack legitimately called this must survive.
    CHECK(!LibraryScanner::isIgnoredDirectoryName("macosx-drums"));
}

void testCountsPacksAndSamples()
{
    ScopedTempDirectory temp { "packs" };

    temp.makeFile("Pack A/kick.wav");
    temp.makeFile("Pack A/snare.wav");
    temp.makeFile("Pack B/Loops/loop1.wav");
    temp.makeFile("Pack B/Loops/Deeper/loop2.aiff");
    temp.makeFile("Pack C/vox.mp3");

    samplebox::LibraryScanner scanner;
    const auto snapshot = scanner.scanTreeNow(temp.path(), nullptr);

    CHECK(snapshot != nullptr);

    // One pack per immediate subdirectory - three, not five. Nested folders
    // inside a pack are part of that pack, not packs of their own.
    CHECK_EQ(snapshot->packs.size(), 3u);
    CHECK_EQ(snapshot->tally.sampleFiles, 5u);

    // Sorted by title, so the order is defined.
    CHECK(snapshot->packs[0].title == "Pack A");
    CHECK(snapshot->packs[1].title == "Pack B");
    CHECK(snapshot->packs[2].title == "Pack C");

    const auto* packB = findPack(*snapshot, "Pack B");
    CHECK(packB != nullptr);

    if (packB != nullptr)
        CHECK_EQ(packB->sampleFiles.size(), 2u); // found at two different depths
}

void testIgnoresNonAudioAndJunk()
{
    ScopedTempDirectory temp { "junk" };

    temp.makeFile("Pack/kick.wav");
    temp.makeFile("Pack/readme.txt");
    temp.makeFile("Pack/cover.png");

    // AppleDouble twin sitting beside a real sample. Before junk filtering was
    // explicit, this was excluded only as a side effect of opening every file.
    temp.makeFile("Pack/._kick.wav");
    temp.makeFile("Pack/.DS_Store");

    // Zero-byte file with a perfectly valid name.
    temp.makeFile("Pack/empty.wav", "");

    samplebox::LibraryScanner scanner;
    const auto snapshot = scanner.scanTreeNow(temp.path(), nullptr);

    CHECK_EQ(snapshot->packs.size(), 1u);
    CHECK_EQ(snapshot->tally.sampleFiles, 1u);

    // ._kick.wav and empty.wav are counted as ignored; readme.txt and cover.png
    // are not, because they never looked like samples in the first place.
    CHECK_EQ(snapshot->tally.ignoredFiles, 2u);

    const auto& pack = snapshot->packs.front();
    CHECK_EQ(pack.sampleFiles.size(), 1u);

    if (!pack.sampleFiles.empty())
        CHECK(pack.sampleFiles.front().filename() == "kick.wav");
}

void testDoesNotDescendIntoMacosxDirectories()
{
    ScopedTempDirectory temp { "macosx" };

    temp.makeFile("Pack/kick.wav");
    temp.makeFile("Pack/__MACOSX/._kick.wav");
    temp.makeFile("Pack/__MACOSX/nested/whatever.wav");

    samplebox::LibraryScanner scanner;
    const auto snapshot = scanner.scanTreeNow(temp.path(), nullptr);

    CHECK_EQ(snapshot->packs.size(), 1u);

    // whatever.wav has an innocent name, so only refusing to descend keeps it
    // out. It is not counted as ignored because it was never looked at.
    CHECK_EQ(snapshot->tally.sampleFiles, 1u);
}

void testSkipsIgnoredTopLevelDirectoriesAsPacks()
{
    ScopedTempDirectory temp { "toplevel" };

    temp.makeFile("Real Pack/kick.wav");
    temp.makeFile("__MACOSX/._something.wav");
    temp.makeFile(".hidden/kick.wav");

    // A loose file at the library root is not a pack and is currently dropped
    // (defect D8, still open). Asserted so the existing behaviour is pinned
    // down rather than assumed.
    temp.makeFile("stray.wav");

    samplebox::LibraryScanner scanner;
    const auto snapshot = scanner.scanTreeNow(temp.path(), nullptr);

    CHECK_EQ(snapshot->packs.size(), 1u);

    if (!snapshot->packs.empty())
        CHECK(snapshot->packs.front().title == "Real Pack");
}

void testResolvesCoverArt()
{
    ScopedTempDirectory temp { "covers" };

    // Preference order: cover beats folder.
    temp.makeFile("Ranked/kick.wav");
    temp.makeFile("Ranked/folder.jpg");
    temp.makeFile("Ranked/cover.png");

    // Case-insensitive match.
    temp.makeFile("Cased/kick.wav");
    temp.makeFile("Cased/Cover.JPG");

    // Sole loose image is used even though it matches no known stem.
    temp.makeFile("Loose/kick.wav");
    temp.makeFile("Loose/Bass House Vol 3.png");

    // Two loose images is ambiguous, so neither is chosen.
    temp.makeFile("Ambiguous/kick.wav");
    temp.makeFile("Ambiguous/one.png");
    temp.makeFile("Ambiguous/two.png");

    // No image at all.
    temp.makeFile("Bare/kick.wav");

    samplebox::LibraryScanner scanner;
    const auto snapshot = scanner.scanTreeNow(temp.path(), nullptr);

    CHECK_EQ(snapshot->packs.size(), 5u);

    const auto* ranked = findPack(*snapshot, "Ranked");
    CHECK(ranked != nullptr);
    if (ranked != nullptr)
        CHECK(ranked->coverArtPath.filename() == "cover.png");

    const auto* cased = findPack(*snapshot, "Cased");
    CHECK(cased != nullptr);
    if (cased != nullptr)
        CHECK(cased->coverArtPath.filename() == "Cover.JPG");

    const auto* loose = findPack(*snapshot, "Loose");
    CHECK(loose != nullptr);
    if (loose != nullptr)
        CHECK(loose->coverArtPath.filename() == "Bass House Vol 3.png");

    const auto* ambiguous = findPack(*snapshot, "Ambiguous");
    CHECK(ambiguous != nullptr);
    if (ambiguous != nullptr)
        CHECK(ambiguous->coverArtPath.empty());

    const auto* bare = findPack(*snapshot, "Bare");
    CHECK(bare != nullptr);
    if (bare != nullptr)
        CHECK(bare->coverArtPath.empty());
}

void testMissingRootYieldsEmptySnapshot()
{
    samplebox::LibraryScanner scanner;
    const auto snapshot = scanner.scanTreeNow("/definitely/not/a/real/path/anywhere", nullptr);

    CHECK(snapshot != nullptr);
    CHECK(snapshot->packs.empty());
    CHECK(!snapshot->tally.cancelled);
}

void testCancellationIsReported()
{
    ScopedTempDirectory temp { "cancel" };

    for (int i = 0; i < 5; ++i)
        temp.makeFile("Pack " + std::to_string(i) + "/kick.wav");

    samplebox::LibraryScanner scanner;

    // Cancel immediately, before the first entry is considered.
    const auto snapshot = scanner.scanTreeNow(temp.path(), [] { return true; });

    CHECK(snapshot != nullptr);
    CHECK(snapshot->tally.cancelled);
    CHECK(snapshot->packs.empty());
}
}

int main()
{
    testExtensionPredicate();
    testIgnoredFilenamePredicate();
    testIgnoredDirectoryPredicate();
    testCountsPacksAndSamples();
    testIgnoresNonAudioAndJunk();
    testDoesNotDescendIntoMacosxDirectories();
    testSkipsIgnoredTopLevelDirectoriesAsPacks();
    testResolvesCoverArt();
    testMissingRootYieldsEmptySnapshot();
    testCancellationIsReported();

    std::printf("%d checks, %d failure(s)\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
