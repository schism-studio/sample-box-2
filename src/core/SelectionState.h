#pragma once

#include "BrowseState.h"

namespace samplebox
{
// Deprecated: BrowseState in BrowseState.h is the unified single source of truth
// for pack selection, sample selection, view mode, and snapshot data.
using SelectionState = BrowseState;
}
