#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace samplebox
{
struct SamplePack
{
    std::string id;
    std::string title;
    std::filesystem::path rootPath;
    std::filesystem::path coverArtPath;
    std::vector<std::filesystem::path> sampleFiles;
};
}
