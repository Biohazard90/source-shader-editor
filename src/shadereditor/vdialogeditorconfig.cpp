
#include "cbase.h"
#include "editorCommon.h"

CDialog_EditorConfig::CDialog_EditorConfig( Panel *parent ) : Frame( parent, "_editor_config" )
{
	SetSizeable( false );
	SetVisible( true );

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	Activate();

	m_pCBut_Draw_Datatypes = new CheckButton( this, "draw_datatypes", "Datatypes" );
	m_pCBut_Draw_Shadows = new CheckButton( this, "draw_shadows", "Shadows" );
	m_pCBut_Draw_Info = new CheckButton( this, "draw_info", "Additional info" );
	m_pCBut_Draw_Tooltip = new CheckButton( this, "draw_tooltip", "Show tooltips on graph" );

	m_pCBut_Compile_Preview = new CheckButton( this, "compile_preview", "Compile enabled" );
	m_pCBut_Compile_Full = new CheckButton( this, "compile_always_full", "Always compile preview + full" );
	m_pCBut_Compile_Publish = new CheckButton( this, "compile_auto_publish", "Auto publish shaders to materials (full compile)" );
	m_pCBut_Compile_WarnOnSave = new CheckButton( this, "warn_on_close_tab", "Warn when closing a tab with unsaved changes" );
	
	m_pCBut_Draw_Datatypes->SetSelected( pEditorRoot->m_bDraw_Datatypes );
	m_pCBut_Draw_Shadows->SetSelected( pEditorRoot->m_bDraw_Shadows );
	m_pCBut_Draw_Info->SetSelected( pEditorRoot->m_bDraw_AllLimits );
	m_pCBut_Draw_Tooltip->SetSelected( pEditorRoot->m_bDoTooltips );

	m_pCBut_Compile_Preview->SetSelected( pEditorRoot->m_bAutoCompile );
	m_pCBut_Compile_Full->SetSelected( pEditorRoot->m_bAutoFullcompile );
	m_pCBut_Compile_Publish->SetSelected( pEditorRoot->m_bAutoShaderPublish );
	m_pCBut_Compile_WarnOnSave->SetSelected( pEditorRoot->m_bWarnOnClose );

	LoadControlSettings("shadereditorui/vgui/dialog_editor_config.res");

	SetTitle( "Editor config", true );

	DoModal();
	SetDeleteSelfOnClose( true );

	m_pCBut_Compile_WarnOnSave->SetEnabled( false );
}

CDialog_EditorConfig::~CDialog_EditorConfig()
{
}

void CDialog_EditorConfig::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( cmd, "save" ) )
	{
		OnSave();
		Close();
	}
	else if ( !Q_stricmp( cmd, "close" ) )
	{
		Close();
	}
}

void CDialog_EditorConfig::PerformLayout()
{
	BaseClass::PerformLayout();
	SetCloseButtonVisible( false );

	SetSize( 400, 325 );
	MoveToCenterOfScreen();
}

void CDialog_EditorConfig::OnSave()
{
	if ( !pEditorRoot->m_bAutoCompile && m_pCBut_Compile_Preview->IsSelected() )
		pEditorRoot->GetSafeFlowgraph()->MakeSolversDirty();

	pEditorRoot->m_bDraw_Datatypes = m_pCBut_Draw_Datatypes->IsSelected();
	pEditorRoot->m_bDraw_Shadows = m_pCBut_Draw_Shadows->IsSelected();
	pEditorRoot->m_bDraw_AllLimits = m_pCBut_Draw_Info->IsSelected();
	pEditorRoot->m_bDoTooltips = m_pCBut_Draw_Tooltip->IsSelected();

	pEditorRoot->m_bAutoCompile = m_pCBut_Compile_Preview->IsSelected();
	pEditorRoot->m_bAutoFullcompile = m_pCBut_Compile_Full->IsSelected();
	pEditorRoot->m_bAutoShaderPublish = m_pCBut_Compile_Publish->IsSelected();
	pEditorRoot->m_bWarnOnClose = m_pCBut_Compile_WarnOnSave->IsSelected();
}
