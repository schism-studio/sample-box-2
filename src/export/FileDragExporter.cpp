#include "FileDragExporter.h"

namespace samplebox::exportmodule
{
bool FileDragExporter::beginDrag(juce::Component& source, const std::filesystem::path& file)
{
    return beginDrag(source, std::vector<std::filesystem::path> { file });
}

bool FileDragExporter::beginDrag(juce::Component& source, const std::vector<std::filesystem::path>& files)
{
    if (files.empty())
        return false;

    juce::StringArray paths;
    for (const auto& file : files)
        paths.add(juce::String(file.string()));

    return juce::DragAndDropContainer::performExternalDragDropOfFiles(paths, false, &source);
}
}
