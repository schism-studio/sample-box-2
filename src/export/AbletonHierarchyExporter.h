#pragma once

#include "../core/SamplePack.h"

#include <filesystem>
#include <string>
#include <vector>

namespace samplebox::exportmodule
{
// Ableton-only export mode: instead of the default curated flat drag,
// reconstructs the pack's on-disk subfolder hierarchy as nested group
// tracks once dropped into Live. Kept as a separate, user-enabled setting
// rather than the universal VST3 export path, since most other DAWs have
// no equivalent concept and no host-agnostic way to build one.
//
// A real implementation needs the Live Object Model bridge that existed
// in the archived Ableton Extension prototype
// (archive/ableton-extension-prototype/src/backend/lom). This class
// currently only builds the in-memory group tree; wiring it to actual
// Live group/audio tracks is a follow-up.
class AbletonHierarchyExporter
{
public:
    struct GroupNode
    {
        std::string name;
        std::vector<std::filesystem::path> files;
        std::vector<GroupNode> children;
    };

    [[nodiscard]] GroupNode buildHierarchy(const SamplePack& pack) const;
};
}
