#include "CoverArtResolver.h"

#include <array>

namespace samplebox
{
std::filesystem::path CoverArtResolver::resolveForPack(const std::filesystem::path& packRoot) const
{
    static constexpr std::array candidates {
        "cover.png", "cover.jpg", "cover.jpeg", "folder.png", "folder.jpg", "folder.jpeg"
    };

    for (const auto* name : candidates)
    {
        const auto candidate = packRoot / name;
        if (std::filesystem::is_regular_file(candidate))
            return candidate;
    }

    return {};
}
}
