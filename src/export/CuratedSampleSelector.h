#pragma once

#include "../core/SamplePack.h"

#include <filesystem>
#include <random>
#include <vector>

namespace samplebox::exportmodule
{
// Builds the curated file list used when a whole pack is dragged out.
//
// Design decision (docs/decisions/0001-vst3-export-redesign.md): since a
// VST3 plugin has no Live Object Model equivalent in any host, full-pack
// export no longer mirrors the folder hierarchy. Instead we pick one
// random eligible file from each available subfolder, then top up with
// additional random files (never repeating) until the deliverable count
// falls in a tweakable [minSamples, maxSamples] range.
class CuratedSampleSelector
{
public:
    struct Options
    {
        int minSamples = 8;
        int maxSamples = 16;
    };

    // NOTE: deliberately two constructors rather than a single
    // `Options options = {}` defaulted parameter. A nested class's default
    // member initializers are not usable in a default argument of the
    // enclosing class, because the enclosing class is still incomplete at
    // that point. MSVC accepts it, but GCC and Clang correctly reject it,
    // which broke any non-MSVC build (and would have broken the Phase 6 CI
    // work). These overloads keep every call site unchanged.
    CuratedSampleSelector();
    explicit CuratedSampleSelector(Options options);

    [[nodiscard]] std::vector<std::filesystem::path> selectForPack(const SamplePack& pack) const;

private:
    Options options;
    mutable std::mt19937 rng { std::random_device {}() };
};
}
