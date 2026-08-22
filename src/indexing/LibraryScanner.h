#pragma once

#include "../core/LibrarySnapshot.h"

#include <filesystem>
#include <functional>

namespace samplebox
{
class LibraryScanner
{
public:
    using Completion = std::function<void(LibrarySnapshot)>;

    void scanAsync(std::filesystem::path root, Completion completion);
};
}
