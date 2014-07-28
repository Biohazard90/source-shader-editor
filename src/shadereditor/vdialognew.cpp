
#include "cbase.h"
#include "editorCommon.h"
#include "vgui_controls/radiobutton.h"
#include <vgui_controls/promptsimple.h>

CDialog_NewCanvas::CDialog_NewCanvas( CNodeView *n, Panel *parent ) : Frame( parent, "newshader" )
{
	pNodeView = n;

	SetSizeable( false );
	SetVisible( true );

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	SetDeleteSelfOnClose( true );
	Activate();

	DoModal();

	m_pRadBut_Sm2 = new RadioButton( this, "rad_1", "SM 2.0b" );
	m_pRadBut_Sm3 = new RadioButton( this, "rad_2", "SM 3.0" );
	m_pRadBut_Sm2->SetSubTabPosition( 1 );
	m_pRadBut_Sm3->SetSubTabPosition( 1 );
	m_pRadBut_Sm3->SetSelected( true );

	LoadControlSettings("shadereditorui/vgui/dialog_newcanvas.res");

	SetTitle( "New shader", true );
}
CDialog_NewCanvas::~CDialog_NewCanvas()
{
}
void CDialog_NewCanvas::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( cmd, "scratch" ) )
	{
		OnDataLoaded(CANVASINIT_SCRATCH);
	}
	else if ( !Q_stricmp( cmd, "postprocess" ) )
	{
		OnDataLoaded(CANVASINIT_POSTPROC);
	}
	else if ( !Q_stricmp( cmd, "model_simple" ) )
	{
		OnDataLoaded(CANVASINIT_MODEL_SIMPLE);
	}
	else if ( !Q_stricmp( cmd, "model_complex" ) )
	{
		OnDataLoaded(CANVASINIT_MODEL_COMPLEX);
	}
	else if ( !Q_stricmp( cmd, "lightmapped_simple" ) )
	{
		OnDataLoaded(CANVASINIT_LIGHTMAPPED_SIMPLE);
	}
	else if ( !Q_stricmp( cmd, "lightmapped_bump" ) )
	{
		OnDataLoaded(CANVASINIT_LIGHTMAPPED_BUMP);
	}
	else if ( !Q_stricmp( cmd, "ppeffect" ) )
	{
		OnDataLoaded(CANVASINIT_PPEFFECT);
	}
	else
		BaseClass::OnCommand( cmd );
}
void CDialog_NewCanvas::OnDataLoaded( int mode )
{
	pEditorRoot->OpenShaderFlowgraph( mode, m_pRadBut_Sm2->IsSelected() );
	CloseModal();
}
void CDialog_NewCanvas::PerformLayout()
{
	BaseClass::PerformLayout();

	SetSize( 400, 400 );
	MoveToCenterOfScreen();
}
