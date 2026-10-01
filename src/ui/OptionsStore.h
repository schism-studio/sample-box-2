#pragma once
#include "../core/AppOptions.h"
#include <juce_data_structures/juce_data_structures.h>

namespace samplebox
{
inline AppOptions loadOptions(juce::PropertiesFile& file)
{
    AppOptions o;
    const int mode = file.getIntValue("defaultViewMode", static_cast<int>(o.defaultViewMode));
    if (mode >= static_cast<int>(PackViewMode::list) && mode <= static_cast<int>(PackViewMode::coverFlow))
        o.defaultViewMode = static_cast<PackViewMode>(mode);
    o.showCoverTitles = file.getBoolValue("showCoverTitles", o.showCoverTitles);
    o.previewVolume = juce::jlimit(0.0f, 1.0f,
        static_cast<float>(file.getDoubleValue("previewVolume", static_cast<double>(o.previewVolume))));
    return o;
}

inline void saveOptions(juce::PropertiesFile& file, const AppOptions& o)
{
    file.setValue("defaultViewMode", static_cast<int>(o.defaultViewMode));
    file.setValue("showCoverTitles", o.showCoverTitles);
    file.setValue("previewVolume", static_cast<double>(o.previewVolume));
    file.saveIfNeeded();
}
}
