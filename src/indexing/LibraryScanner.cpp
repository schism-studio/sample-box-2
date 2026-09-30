#include "LibraryScanner.h"

#include <juce_events/juce_events.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <memory>
#include <string>

namespace samplebox
{
namespace
{
constexpr std::array kSupportedExtensions {
    ".wav", ".aif", ".aiff", ".mp3", ".flac"
};

// Directory names that appear inside real sample packs and contain nothing
// playable. __MACOSX in particular is created by macOS's zip tooling and is
// full of AppleDouble stubs that carry the *same* extensions as the real
// samples sitting next to them, so descending into it produces a pack with
// twice as many entries as it has samples, half of which cannot be played.
constexpr std::array kIgnoredDirectoryNames {
    "__macosx", "$recycle.bin", "system volume information"
};

std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}
}

LibraryScanner::LibraryScanner() : juce::Thread("SampleBox.LibraryScanner") {}

LibraryScanner::~LibraryScanner()
{
    cancelScan();
}

void LibraryScanner::scanAsync(std::filesystem::path root, Completion completion)
{
    cancelScan();
    rootPath = std::move(root);
    onComplete = std::move(completion);
    startThread(juce::Thread::Priority::background);
}

void LibraryScanner::cancelScan()
{
    signalThreadShouldExit();
    stopThread(5000);
}

bool LibraryScanner::hasSupportedAudioExtension(const std::filesystem::path& path)
{
    const auto extension = toLower(path.extension().string());
    return std::find(kSupportedExtensions.begin(), kSupportedExtensions.end(), extension)
        != kSupportedExtensions.end();
}

bool LibraryScanner::isIgnoredFilename(const std::string& filename)
{
    // AppleDouble resource forks. A pack that has been zipped on a Mac and
    // unzipped on Windows is full of these: for every kick.wav there is a
    // ._kick.wav, which is a few hundred bytes of metadata carrying the .wav
    // extension. Nothing but the leading "._" distinguishes them by name.
    //
    // These used to be filtered out as a side effect of building an
    // AudioFormatReader for every candidate file - the reader failed, so the
    // file was dropped. Now that the scan does not open files, they have to be
    // excluded explicitly or every Mac-authored pack doubles in apparent size.
    if (filename.rfind("._", 0) == 0)
        return true;

    // Dotfiles, including .DS_Store. Never user content.
    if (filename.rfind('.', 0) == 0)
        return true;

    return false;
}

bool LibraryScanner::isIgnoredDirectoryName(const std::string& filename)
{
    if (filename.rfind('.', 0) == 0)
        return true;

    const auto lowercase = toLower(filename);
    return std::find(kIgnoredDirectoryNames.begin(), kIgnoredDirectoryNames.end(), lowercase)
        != kIgnoredDirectoryNames.end();
}

SamplePack LibraryScanner::buildPackFromDirectory(const std::filesystem::path& packRoot,
                                                  ScanTally& tally,
                                                  const std::function<bool()>& shouldCancel)
{
    SamplePack pack;
    pack.id = packRoot.string();
    pack.title = packRoot.filename().string();
    pack.rootPath = packRoot;
    pack.coverArtPath = coverArtResolver.resolveForPack(packRoot);

    std::error_code ec;
    std::filesystem::recursive_directory_iterator it(
        packRoot, std::filesystem::directory_options::skip_permission_denied, ec);
    const std::filesystem::recursive_directory_iterator end;

    if (ec)
    {
        ++tally.unreadableDirectories;
        return pack;
    }

    while (it != end)
    {
        if (shouldCancel && shouldCancel())
        {
            tally.cancelled = true;
            break;
        }

        const auto entry = *it;

        std::error_code entryEc;
        if (entry.is_directory(entryEc) && !entryEc)
        {
            if (isIgnoredDirectoryName(entry.path().filename().string()))
                it.disable_recursion_pending();
        }
        else if (entry.is_regular_file(entryEc) && !entryEc)
        {
            const auto filename = entry.path().filename().string();

            if (hasSupportedAudioExtension(entry.path()))
            {
                if (isIgnoredFilename(filename))
                {
                    ++tally.ignoredFiles;
                }
                else
                {
                    // A zero-byte file has a valid name and no content. Cheap to
                    // test here because directory_entry caches the size from the
                    // same OS call that enumerated the directory, so this costs
                    // no extra syscall on Windows.
                    std::error_code sizeEc;
                    if (entry.file_size(sizeEc) == 0 && !sizeEc)
                        ++tally.ignoredFiles;
                    else
                        pack.sampleFiles.push_back(entry.path());
                }
            }
        }

        // Advance with a *separate* error_code, and clear it.
        //
        // This loop used to be `for (; it != end && !ec; it.increment(ec))`,
        // sharing one error_code between the increment and the loop condition.
        // The effect was that the first unreadable subdirectory anywhere in a
        // pack did not skip that subdirectory - it ended the entire scan of
        // that pack, silently, with whatever had been collected so far. On a
        // library where one folder had awkward permissions, the user would see
        // a plausible-looking but arbitrarily truncated pack and have no way to
        // know. Recording it in the tally is what makes it visible.
        std::error_code advanceEc;
        it.increment(advanceEc);

        if (advanceEc)
        {
            ++tally.unreadableDirectories;

            // increment() failing means the iterator's position is not
            // meaningful, so there is nothing safe to continue from.
            break;
        }
    }

    std::sort(pack.sampleFiles.begin(), pack.sampleFiles.end());
    tally.sampleFiles += pack.sampleFiles.size();
    return pack;
}

std::shared_ptr<LibrarySnapshot> LibraryScanner::scanTreeNow(
    const std::filesystem::path& root,
    const std::function<bool()>& shouldCancel)
{
    // Built as a mutable local, then published as immutable shared state. The
    // snapshot is never touched again after this function returns.
    auto snapshot = std::make_shared<LibrarySnapshot>();
    snapshot->generatedAt = std::chrono::system_clock::now();

    std::error_code ec;
    if (std::filesystem::is_directory(root, ec) && !ec)
    {
        std::filesystem::directory_iterator it(root, ec);
        const std::filesystem::directory_iterator end;

        if (ec)
            ++snapshot->tally.unreadableDirectories;

        while (!ec && it != end)
        {
            if (shouldCancel && shouldCancel())
            {
                snapshot->tally.cancelled = true;
                break;
            }

            std::error_code entryEc;
            if (it->is_directory(entryEc) && !entryEc
                && !isIgnoredDirectoryName(it->path().filename().string()))
                snapshot->packs.push_back(
                    buildPackFromDirectory(it->path(), snapshot->tally, shouldCancel));

            // Same fix as in buildPackFromDirectory: one unreadable entry at
            // the top level used to end the whole library scan.
            std::error_code advanceEc;
            it.increment(advanceEc);

            if (advanceEc)
            {
                ++snapshot->tally.unreadableDirectories;
                break;
            }
        }

        std::sort(snapshot->packs.begin(), snapshot->packs.end(),
                  [](const SamplePack& a, const SamplePack& b) { return a.title < b.title; });
    }

    return snapshot;
}

void LibraryScanner::run()
{
    auto snapshot = scanTreeNow(rootPath, [this] { return threadShouldExit(); });

    // A cancelled scan is discarded rather than delivered. scanAsync() cancels
    // before starting, so the only reason to be here is that a *newer* scan is
    // about to run, and publishing a half-built library first would show the
    // user a briefly wrong browser.
    if (threadShouldExit())
        return;

    // Hand the snapshot over by pointer. Previously this deep-copied the whole
    // snapshot into `result` and then again into the lambda's captured copy,
    // which for a large library means copying every path in every pack twice on
    // the scanner thread before the UI ever sees it.
    juce::MessageManager::callAsync(
        [completion = onComplete, result = LibrarySnapshotPtr(std::move(snapshot))]() mutable {
            if (completion)
                completion(std::move(result));
        });
}
}
