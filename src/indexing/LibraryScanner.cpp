#include "LibraryScanner.h"

#include <juce_events/juce_events.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <memory>

namespace samplebox
{
namespace
{
constexpr std::array kSupportedExtensions {
    ".wav", ".aif", ".aiff", ".mp3", ".flac"
};

std::string toLowerExtension(const std::filesystem::path& path)
{
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}
}

LibraryScanner::LibraryScanner() : juce::Thread("SampleBox.LibraryScanner")
{
    formatManager.registerBasicFormats();
}

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

bool LibraryScanner::isSupportedAudioFile(const std::filesystem::path& path)
{
    const auto ext = toLowerExtension(path);
    if (std::find(kSupportedExtensions.begin(), kSupportedExtensions.end(), ext) == kSupportedExtensions.end())
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(juce::File(path.string())));
    return reader != nullptr;
}

SamplePack LibraryScanner::buildPackFromDirectory(const std::filesystem::path& packRoot)
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

    for (; it != end && !ec; it.increment(ec))
    {
        if (threadShouldExit())
            break;

        const auto& entry = *it;
        std::error_code fileEc;
        if (!entry.is_regular_file(fileEc) || fileEc)
            continue;

        if (isSupportedAudioFile(entry.path()))
            pack.sampleFiles.push_back(entry.path());
    }

    std::sort(pack.sampleFiles.begin(), pack.sampleFiles.end());
    return pack;
}

void LibraryScanner::run()
{
    // Built as a mutable local, then published as immutable shared state. The
    // snapshot is never touched again after this function returns.
    auto snapshot = std::make_shared<LibrarySnapshot>();
    snapshot->generatedAt = std::chrono::system_clock::now();

    std::error_code ec;
    if (std::filesystem::is_directory(rootPath, ec) && !ec)
    {
        std::filesystem::directory_iterator it(rootPath, ec);
        const std::filesystem::directory_iterator end;

        for (; it != end && !ec; it.increment(ec))
        {
            if (threadShouldExit())
                break;

            std::error_code dirEc;
            if (it->is_directory(dirEc) && !dirEc)
                snapshot->packs.push_back(buildPackFromDirectory(it->path()));
        }

        std::sort(snapshot->packs.begin(), snapshot->packs.end(),
                  [](const SamplePack& a, const SamplePack& b) { return a.title < b.title; });
    }

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
