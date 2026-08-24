#pragma once

#include "../core/LibrarySnapshot.h"
#include "CoverArtResolver.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

#include <filesystem>
#include <functional>

namespace samplebox
{
// Background library scanner. Each immediate subdirectory of the chosen
// root is treated as one SamplePack; all supported audio files found
// anywhere beneath a pack directory (recursively) belong to that pack.
// Ported from the working Antigravity::DirectoryScanner prototype, but
// reshaped to produce an immutable LibrarySnapshot instead of a flat file
// list, and to run as a small self-contained class with no Listener
// interface — callers get a single completion callback on the message
// thread once the scan finishes.
class LibraryScanner final : public juce::Thread
{
public:
    // The snapshot is delivered as shared immutable state, not by value: it
    // can hold hundreds of packs with thousands of paths each, and the UI
    // objects that display it need to keep it alive independently of whoever
    // received it first.
    using Completion = std::function<void(LibrarySnapshotPtr)>;

    LibraryScanner();
    ~LibraryScanner() override;

    // Cancels any scan in progress, then starts scanning `root` on a
    // background thread. `completion` is invoked on the JUCE message thread
    // once the scan finishes (or immediately with an empty snapshot if
    // `root` is not a directory).
    void scanAsync(std::filesystem::path root, Completion completion);

    // Cancels a scan in progress, if any. Safe to call when idle.
    void cancelScan();

    bool isScanning() const { return isThreadRunning(); }

    void run() override;

private:
    bool isSupportedAudioFile(const std::filesystem::path& path);
    SamplePack buildPackFromDirectory(const std::filesystem::path& packRoot);

    std::filesystem::path rootPath;
    Completion onComplete;

    juce::AudioFormatManager formatManager;
    CoverArtResolver coverArtResolver;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibraryScanner)
};
}
