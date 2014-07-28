
#include "cbase.h"
#include "editorCommon.h"


CDialog_PPEConfig::CDialog_PPEConfig( Panel *parent, CNodeView *pNodeView ) : Frame( parent, "ppe_config" )
{
	m_pNodeView = pNodeView;
	SetSizeable( false );
	SetVisible( true );

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	Activate();

	m_pCheck_AutoUpdateFBCopy = new CheckButton( this, "autoupdatefb", "Auto update FB copy tex" );

	LoadControlSettings("shadereditorui/vgui/dialog_ppe_config.res");

	m_pCheck_AutoUpdateFBCopy->SetSelected( pNodeView->AccessPPEConfig()->bDoAutoUpdateFBCopy );

	SetSize( 280, 220 );
	SetTitle( "PPE settings", true );

	DoModal();

	SetDeleteSelfOnClose( true );
}

CDialog_PPEConfig::~CDialog_PPEConfig()
{
}

void CDialog_PPEConfig::OnCommand( const char *cmd )
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

void CDialog_PPEConfig::OnSave()
{
	m_pNodeView->AccessPPEConfig()->bDoAutoUpdateFBCopy = m_pCheck_AutoUpdateFBCopy->IsSelected();
}