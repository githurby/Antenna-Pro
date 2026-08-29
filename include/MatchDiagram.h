#pragma once
#include <windows.h>
#include "AntennaModel.h"

// Opens an independent window with the calculated matching-network construction diagram.
void showMatchDiagram(HWND owner, const ant::DesignInput& input, const ant::DesignResult& result);
