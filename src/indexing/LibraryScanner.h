#pragma once

#include "../core/LibrarySnapshot.h"
#include "CoverArtResolver.h"

#include <juce_core/juce_core.h>

#include <filesystem>
#include <functional>
#include <memory>

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
//
// The scan is deliberately *optimistic*: a file is treated as a sample based
// on its name alone, and is never opened. Confirming that a file really
// decodes means opening it and parsing a header, which at library scale is
// tens of thousands of file opens for information the audition path has to
// re-establish anyway. Playback is therefore the layer that must tolerate a
// file that turns out to be unreadable, not the scan.
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

    // Walks `root` on the *calling* thread and returns the resulting snapshot.
    // `shouldCancel` is polled between entries and may be null.
    //
    // Split out from run() so the whole traversal - filtering, tallying, error
    // recovery - is testable without a background thread or a running JUCE
    // message loop. run() is then only responsible for threading and delivery.
    [[nodiscard]] std::shared_ptr<LibrarySnapshot> scanTreeNow(
        const std::filesystem::path& root,
        const std::function<bool()>& shouldCancel);

    // Exposed for testing: these are pure name-based predicates with no
    // filesystem access, and they encode assumptions about real-world sample
    // packs that are worth being able to assert on directly.

    // True when `path` carries an extension Sample Box can play. Says nothing
    // about whether the file is readable or genuinely audio.
    [[nodiscard]] static bool hasSupportedAudioExtension(const std::filesystem::path& path);

    // True for filenames that must never become samples even when they carry a
    // supported extension.
    [[nodiscard]] static bool isIgnoredFilename(const std::string& filename);

    // True for directories that must not be descended into, and must not
    // themselves be treated as packs.
    [[nodiscard]] static bool isIgnoredDirectoryName(const std::string& filename);

private:
    SamplePack buildPackFromDirectory(const std::filesystem::path& packRoot,
                                      ScanTally& tally,
                                      const std::function<bool()>& shouldCancel);

    std::filesystem::path rootPath;
    Completion onComplete;

    CoverArtResolver coverArtResolver;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibraryScanner)
};
}
