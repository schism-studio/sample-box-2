#include "AbletonHierarchyExporter.h"

#include <system_error>

namespace samplebox::exportmodule
{
namespace
{
AbletonHierarchyExporter::GroupNode buildNode(const std::filesystem::path& dir)
{
    AbletonHierarchyExporter::GroupNode node;
    node.name = dir.filename().string();

    std::error_code ec;
    std::filesystem::directory_iterator it(dir, ec);
    const std::filesystem::directory_iterator end;

    for (; it != end && !ec; it.increment(ec))
    {
        std::error_code entryEc;
        if (it->is_directory(entryEc) && !entryEc)
        {
            node.children.push_back(buildNode(it->path()));
        }
        else if (it->is_regular_file(entryEc) && !entryEc)
        {
            node.files.push_back(it->path());
        }
    }

    return node;
}
}

AbletonHierarchyExporter::GroupNode AbletonHierarchyExporter::buildHierarchy(const SamplePack& pack) const
{
    return buildNode(pack.rootPath);
}
}
