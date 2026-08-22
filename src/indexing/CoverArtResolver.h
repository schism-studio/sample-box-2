#pragma once

#include <filesystem>

namespace samplebox
{
class CoverArtResolver
{
public:
    [[nodiscard]] std::filesystem::path resolveForPack(const std::filesystem::path& packRoot) const;
};
}
