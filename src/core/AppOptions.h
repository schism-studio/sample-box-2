#pragma once
#include "BrowseState.h"

namespace samplebox
{
// Global user options. Persisted by the host (standalone + plug-in share one PropertiesFile).
struct AppOptions
{
    PackViewMode defaultViewMode = PackViewMode::coverFlow;
    bool showCoverTitles = true;      // matches the previous always-on behaviour
    float previewVolume = 1.0f;       // linear gain, 0..1
};
}
