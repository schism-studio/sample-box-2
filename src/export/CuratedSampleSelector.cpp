#include "CuratedSampleSelector.h"

#include <algorithm>
#include <map>

namespace samplebox::exportmodule
{
CuratedSampleSelector::CuratedSampleSelector() : options(Options {}) {}

CuratedSampleSelector::CuratedSampleSelector(Options optionsIn) : options(optionsIn) {}

std::vector<std::filesystem::path> CuratedSampleSelector::selectForPack(const SamplePack& pack) const
{
    // Group sample files by their immediate parent directory ("subfolder").
    std::map<std::filesystem::path, std::vector<std::filesystem::path>> bySubfolder;
    for (const auto& file : pack.sampleFiles)
        bySubfolder[file.parent_path()].push_back(file);

    std::vector<std::filesystem::path> selected;
    std::vector<std::filesystem::path> remaining;

    // Step 1: one random file per available subfolder.
    for (auto& [folder, files] : bySubfolder)
    {
        std::shuffle(files.begin(), files.end(), rng);
        selected.push_back(files.front());
        remaining.insert(remaining.end(), files.begin() + 1, files.end());
    }

    // Step 2: if there weren't enough subfolders to reach the minimum, top
    // up with additional random files (never repeating a selected one)
    // until we hit the target, bounded by maxSamples.
    std::shuffle(remaining.begin(), remaining.end(), rng);

    const int target = std::clamp((int) selected.size() < options.minSamples ? options.minSamples
                                                                               : (int) selected.size(),
                                    options.minSamples, options.maxSamples);

    for (const auto& file : remaining)
    {
        if ((int) selected.size() >= target)
            break;
        selected.push_back(file);
    }

    if ((int) selected.size() > options.maxSamples)
        selected.resize((std::size_t) options.maxSamples);

    return selected;
}
}
