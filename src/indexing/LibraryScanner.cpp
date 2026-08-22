#include "LibraryScanner.h"

namespace samplebox
{
void LibraryScanner::scanAsync(std::filesystem::path, Completion)
{
    // Skeleton: perform traversal on a worker thread and dispatch an immutable
    // LibrarySnapshot back to the UI layer in the indexing milestone.
}
}
