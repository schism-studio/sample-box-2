#pragma once

#include "SamplePack.h"

#include <chrono>
#include <vector>

namespace samplebox
{
struct LibrarySnapshot
{
    std::vector<SamplePack> packs;
    std::chrono::system_clock::time_point generatedAt;
};
}
