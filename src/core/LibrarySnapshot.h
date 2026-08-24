#pragma once

#include "SamplePack.h"

#include <chrono>
#include <memory>
#include <vector>

namespace samplebox
{
// What the scan actually managed to do, as opposed to what it was asked to
// do. Carried alongside the results because a scan that quietly skipped half
// a library looks exactly like a library that is half that size, and the user
// is the only one who can tell the difference.
struct ScanTally
{
    std::size_t sampleFiles = 0;

    // Directories that could not be read or enumerated (permissions, a
    // disconnected network share, a path exceeding the OS limit). Their
    // contents are missing from the snapshot.
    std::size_t unreadableDirectories = 0;

    // Files skipped because they carry a supported extension but are not
    // usable samples - AppleDouble stubs, zero-byte files, dotfiles.
    std::size_t ignoredFiles = 0;

    // True when the scan stopped early because it was cancelled, which means
    // the snapshot is incomplete by design rather than by failure.
    bool cancelled = false;
};

struct LibrarySnapshot
{
    std::vector<SamplePack> packs;
    std::chrono::system_clock::time_point generatedAt;
    ScanTally tally;
};

// A snapshot is produced once by the scanner and thereafter never mutated,
// so it is passed around as shared immutable state rather than copied or
// moved between owners. Every UI object that points into a snapshot's packs
// holds one of these, which is what keeps those pointers valid for as long
// as they are in use, no matter what order the owners are torn down in.
using LibrarySnapshotPtr = std::shared_ptr<const LibrarySnapshot>;
}
