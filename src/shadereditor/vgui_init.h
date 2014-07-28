#ifndef VGUI_EDITOR_INIT
#define VGUI_EDITOR_INIT
#ifdef _WIN32
#pragma once
#endif

#include "interface.h"
#include <vgui/VGUI.h>

namespace vgui
{
	class Panel;
}

bool VGui_Editor_Startup( CreateInterfaceFn appSystemFactory );
void VGui_Editor_Shutdown( void );
void VGui_Editor_CreateGlobalPanels( bool bEditingMode );

#endif