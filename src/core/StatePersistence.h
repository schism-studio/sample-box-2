#pragma once

#include "BrowseState.h"

#include <juce_core/juce_core.h>

#include <memory>
#include <string>

namespace samplebox
{
// Serializes and restores plugin instance state and user settings across DAW
// saves/reloads using a standard JUCE XmlElement representation.
class StatePersistence
{
public:
    static constexpr int CURRENT_STATE_VERSION = 1;

    struct InstanceState
    {
        int version = CURRENT_STATE_VERSION;
        juce::String sampleLibraryPath;
        PackViewMode packViewMode = PackViewMode::coverFlow;
        std::string focusedPackId;
        std::string selectedSamplePath;
        double thumbnailZoomGrid = 0.5;
        double thumbnailZoomList = 0.5;
        double thumbnailZoomCoverFlow = 0.5;
        int randomDragSampleCount = 5;
        bool showCoverTitles = false;
    };

    static std::unique_ptr<juce::XmlElement> createXmlFromState(const InstanceState& state)
    {
        auto xml = std::make_unique<juce::XmlElement>("SAMPLEBOX_STATE");
        xml->setAttribute("version", state.version);
        xml->setAttribute("sampleLibraryPath", state.sampleLibraryPath);
        xml->setAttribute("packViewMode", static_cast<int>(state.packViewMode));
        xml->setAttribute("focusedPackId", juce::String(state.focusedPackId));
        xml->setAttribute("selectedSamplePath", juce::String(state.selectedSamplePath));
        xml->setAttribute("thumbnailZoomGrid", state.thumbnailZoomGrid);
        xml->setAttribute("thumbnailZoomList", state.thumbnailZoomList);
        xml->setAttribute("thumbnailZoomCoverFlow", state.thumbnailZoomCoverFlow);
        xml->setAttribute("randomDragSampleCount", state.randomDragSampleCount);
        xml->setAttribute("showCoverTitles", state.showCoverTitles);
        return xml;
    }

    static InstanceState restoreStateFromXml(const juce::XmlElement& xml)
    {
        InstanceState state;
        if (!xml.hasTagName("SAMPLEBOX_STATE"))
            return state;

        state.version = xml.getIntAttribute("version", CURRENT_STATE_VERSION);
        state.sampleLibraryPath = xml.getStringAttribute("sampleLibraryPath", {});
        
        const auto viewModeInt = xml.getIntAttribute("packViewMode", static_cast<int>(PackViewMode::coverFlow));
        if (viewModeInt >= static_cast<int>(PackViewMode::list) && viewModeInt <= static_cast<int>(PackViewMode::coverFlow))
            state.packViewMode = static_cast<PackViewMode>(viewModeInt);

        state.focusedPackId = xml.getStringAttribute("focusedPackId", {}).toStdString();
        state.selectedSamplePath = xml.getStringAttribute("selectedSamplePath", {}).toStdString();
        state.thumbnailZoomGrid = xml.getDoubleAttribute("thumbnailZoomGrid", 0.5);
        state.thumbnailZoomList = xml.getDoubleAttribute("thumbnailZoomList", 0.5);
        state.thumbnailZoomCoverFlow = xml.getDoubleAttribute("thumbnailZoomCoverFlow", 0.5);
        state.randomDragSampleCount = xml.getIntAttribute("randomDragSampleCount", 5);
        state.showCoverTitles = xml.getBoolAttribute("showCoverTitles", false);

        return state;
    }

    static void writeToMemoryBlock(const InstanceState& state, juce::MemoryBlock& destData)
    {
        const auto xml = createXmlFromState(state);
        if (xml != nullptr)
            juce::AudioProcessor::copyXmlToBinary(*xml, destData);
    }

    static InstanceState readFromMemoryBlock(const void* data, int sizeInBytes)
    {
        const auto xml = juce::AudioProcessor::getXmlFromBinary(data, sizeInBytes);
        if (xml != nullptr)
            return restoreStateFromXml(*xml);

        return {};
    }
};
}
