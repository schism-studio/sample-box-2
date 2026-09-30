#include "CoverArtResolver.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>

namespace samplebox
{
namespace
{
// Only formats juce::ImageFileFormat can actually decode. Listing anything
// else here would resolve a cover path that ArtworkCache then silently fails
// to load, showing a blank card instead of falling back to the placeholder.
constexpr std::array kImageExtensions { ".png", ".jpg", ".jpeg", ".gif" };

// Preferred stems in priority order. A pack with both cover.png and
// folder.jpg should use cover.png, so order here is meaningful.
constexpr std::array kPreferredStems { "cover", "folder", "artwork", "front", "art" };

std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool isImageExtension(const std::string& lowercaseExtension)
{
    return std::find(kImageExtensions.begin(), kImageExtensions.end(), lowercaseExtension)
        != kImageExtensions.end();
}

// Lower rank wins. Returns kImageExtensions.size() for "an image, but not a
// recognised name", which is still useful as a last resort.
std::size_t rankForStem(const std::string& lowercaseStem)
{
    for (std::size_t i = 0; i < kPreferredStems.size(); ++i)
        if (lowercaseStem == kPreferredStems[i])
            return i;

    return kPreferredStems.size();
}
}

std::filesystem::path CoverArtResolver::resolveForPack(const std::filesystem::path& packRoot) const
{
    // One directory listing rather than a stat per candidate name.
    //
    // The previous version probed six fixed filenames with the *throwing*
    // overload of std::filesystem::is_regular_file. That threw
    // std::filesystem_error out of the scanner thread on an unreadable
    // directory, with no handler anywhere above it, which terminates the
    // process. Everything here uses the error_code overloads.
    //
    // Listing once also makes the match case-insensitive. The old exact-match
    // probe only worked because Windows filesystems happen to be
    // case-insensitive; "Cover.jpg" would have been missed anywhere else, and
    // sample packs are not consistent about capitalisation.
    std::error_code ec;
    std::filesystem::directory_iterator it(packRoot, ec);
    const std::filesystem::directory_iterator end;

    if (ec)
        return {};

    std::filesystem::path best;
    auto bestRank = kPreferredStems.size() + 1;
    int looseImageCount = 0;
    std::filesystem::path looseImage;

    for (; it != end; it.increment(ec))
    {
        if (ec)
        {
            // One bad entry must not abandon the rest of the directory.
            ec.clear();
            continue;
        }

        std::error_code entryEc;
        if (!it->is_regular_file(entryEc) || entryEc)
            continue;

        const auto filename = it->path().filename().string();

        // AppleDouble sidecars (._cover.png) and dotfiles are never artwork.
        if (filename.rfind("._", 0) == 0 || filename.rfind('.', 0) == 0)
            continue;

        const auto extension = toLower(it->path().extension().string());
        if (!isImageExtension(extension))
            continue;

        const auto rank = rankForStem(toLower(it->path().stem().string()));

        if (rank < kPreferredStems.size())
        {
            if (rank < bestRank)
            {
                bestRank = rank;
                best = it->path();
            }
        }
        else
        {
            ++looseImageCount;
            looseImage = it->path();
        }
    }

    if (!best.empty())
        return best;

    // Fallback: a pack whose only root-level image is "Bass House Vol 3.png"
    // clearly means that to be the cover, and real packs are named that way far
    // more often than they contain a file literally called cover.png. Only
    // taken when there is exactly one candidate - with two or more there is no
    // basis for choosing, and guessing wrong is worse than the placeholder.
    if (looseImageCount == 1)
        return looseImage;

    return {};
}
}
