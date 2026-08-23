#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <filesystem>
#include <vector>

namespace samplebox::exportmodule
{
// Starts a native OS file drag (single file, or the curated multi-file
// selection for a whole pack) from any component, so users can drop
// samples straight into their DAW's arrangement or browser. This is the
// default, cross-DAW export path — it replaces the LOM-based hierarchy
// export from the original Ableton Extension prototype for everything
// except the Ableton-only group-track mode (see AbletonHierarchyExporter).
class FileDragExporter
{
public:
    static bool beginDrag(juce::Component& source, const std::filesystem::path& file);
    static bool beginDrag(juce::Component& source, const std::vector<std::filesystem::path>& files);
};
}
