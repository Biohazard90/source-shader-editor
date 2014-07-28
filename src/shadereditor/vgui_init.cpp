
#include "cbase.h"
//#include "vgui_int.h"
#include "ienginevgui.h"
#include <vgui/isurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include "tier0/vprof.h"
#include <vgui_controls/Panel.h>
#include <KeyValues.h>
#include "FileSystem.h"
#ifndef SHADER_EDITOR_DLL_2006
#include "matsys_controls/matsyscontrols.h"
#endif
#include "vguimatsurface/IMatSystemSurface.h"
#include "vgui/isystem.h"
#include <vgui/ILocalize.h>
#include <tier3/tier3.h>
#include <steam/steam_api.h>
#include "vgui_controls/animationcontroller.h"

#include "vEditorRoot.h"
#include "editorCommon.h"

using namespace vgui;

#include <vgui/IInputInternal.h>
#include <vgui_controls/Controls.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

vgui::IInputInternal *g_InputInternal = NULL;
CEditorRoot *pShaderEditor = NULL;

bool VGui_Editor_Startup( CreateInterfaceFn appSystemFactory )
{
	enginevgui = (IEngineVGui *)appSystemFactory( VENGINE_VGUI_VERSION, NULL );
	if ( !enginevgui )
		return false;

#if 0
	g_pVGuiSurface = (vgui::ISurface*)appSystemFactory( VGUI_SURFACE_INTERFACE_VERSION, NULL );
	if ( !g_pVGuiSurface )
		return false;
	g_pVGuiSurface->Connect( appSystemFactory );

	g_pMatSystemSurface = (IMatSystemSurface *)appSystemFactory(MAT_SYSTEM_SURFACE_INTERFACE_VERSION, NULL);
	if ( !g_pMatSystemSurface )
		return false;
	g_pMatSystemSurface->Connect( appSystemFactory );

	g_pVGuiInput = (IInput *)appSystemFactory(VGUI_INPUT_INTERFACE_VERSION, NULL);
	if ( !g_pVGuiInput )
		return false;

	g_pVGui = (IVGui *)appSystemFactory(VGUI_IVGUI_INTERFACE_VERSION, NULL);
	if ( !g_pVGui )
		return false;
	g_pVGui->Connect( appSystemFactory );

	g_pVGuiPanel = (IPanel *)appSystemFactory(VGUI_PANEL_INTERFACE_VERSION, NULL);
	if ( !g_pVGuiPanel )
		return false;

#ifndef SHADER_EDITOR_DLL_SWARM
	g_pVGuiLocalize = ( vgui::ILocalize *)appSystemFactory(VGUI_LOCALIZE_INTERFACE_VERSION, NULL);
	if ( !g_pVGuiLocalize )
		return false;
#endif

	g_pVGuiSchemeManager = (ISchemeManager *)appSystemFactory(VGUI_SCHEME_INTERFACE_VERSION, NULL);
	if ( !g_pVGuiSchemeManager )
		return false;

	g_pVGuiSystem = (ISystem *)appSystemFactory(VGUI_SYSTEM_INTERFACE_VERSION, NULL);
	if ( !g_pVGuiSystem )
		return false;

	//g_InputInternal = (IInputInternal *)appSystemFactory( VGUI_INPUTINTERNAL_INTERFACE_VERSION,  NULL );
	//if ( !g_InputInternal )
	//	return false;
#endif

	if ( !vgui::VGui_InitInterfacesList( "ShaderEditorUI", &appSystemFactory, 1 ) )
		return false;

	if ( g_pVGuiSurface == NULL ||
		g_pMatSystemSurface == NULL ||
		g_pVGuiInput == NULL ||
		g_pVGui == NULL ||
		g_pVGuiPanel == NULL ||
		g_pVGuiLocalize == NULL ||
		g_pVGuiSchemeManager == NULL ||
		g_pVGuiSystem == NULL )
		return false;

#ifndef SHADER_EDITOR_DLL_2006
	if ( !vgui::VGui_InitMatSysInterfacesList( "ShaderEditorUI", &appSystemFactory, 1 ) )
		return false;
#endif

	return true;
}

#define SEDITSCHEME_PATH "shadereditorui/ShaderEditorScheme.res"
#define SEDITSCHEME_TAG "ShaderEditorScheme"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGui_Editor_CreateGlobalPanels( bool bEditingMode )
{
	//char animFileName[MAX_PATH];
	//Q_snprintf( animFileName, MAX_PATH, "%s\\shadereditorui\\SEditUI.txt" );
	//::GetAnimationController()->SetScriptFile( pShaderEditor->GetVPanel(), animFileName );

	if ( bEditingMode )
	{
		HScheme seditScheme = g_pVGuiSchemeManager->LoadSchemeFromFile( SEDITSCHEME_PATH, SEDITSCHEME_TAG );

		if ( !pShaderEditor )
			pShaderEditor = new CEditorRoot("editor");
		if ( pShaderEditor )
		{
			pShaderEditor->SetScheme( seditScheme );
			pShaderEditor->MakeReadyForUse();
			pShaderEditor->InvalidateLayout( true, true );

			Assert( pShaderEditor->GetSafeFlowgraph() );

			pShaderEditor->GetSafeFlowgraph()->InitCanvas();
			pShaderEditor->GetSafeFlowgraph()->ResetView_User( true );

			pShaderEditor->LoadLayout();

			pShaderEditor->GetSafeFlowgraph()->FlushHistory();
			pShaderEditor->GetSafeFlowgraph()->UnDirtySave();
		}
	}
}

void VGui_Editor_Shutdown()
{
	if ( pShaderEditor )
	{
		pShaderEditor->SaveLayout();
		pShaderEditor->SetParent( (Panel*)NULL );
		delete pShaderEditor;
		pShaderEditor = NULL;
	}

	DestroyDirectoryStrings();

	vgui::ivgui()->RunFrame();

	//g_pVGui->Disconnect();
	//g_pMatSystemSurface->Disconnect();
	//g_pVGuiSurface->Disconnect();
}