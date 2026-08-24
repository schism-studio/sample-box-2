#pragma once

#include "SamplePack.h"

#include <chrono>
#include <memory>
#include <vector>

namespace samplebox
{
struct LibrarySnapshot
{
    std::vector<SamplePack> packs;
    std::chrono::system_clock::time_point generatedAt;
};

// A snapshot is produced once by the scanner and thereafter never mutated,
// so it is passed around as shared immutable state rather than copied or
// moved between owners. Every UI object that points into a snapshot's packs
// holds one of these, which is what keeps those pointers valid for as long
// as they are in use, no matter what order the owners are torn down in.
using LibrarySnapshotPtr = std::shared_ptr<const LibrarySnapshot>;
}
