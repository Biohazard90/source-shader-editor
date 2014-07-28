
#include "cbase.h"
#include "editorCommon.h"


CDialog_PPEPrecache::CDialog_PPEPrecache( Panel *parent ) : BaseClass( parent, NULL, "dialog_precache_ppeffect" )
{
	SetSizeable( false );
	SetVisible( true );

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	Activate();

	m_pList_Effects = new PanelListPanel( this, "effect_list" );
	m_pList_Effects->SetNumColumns( 4 );
	m_pList_Effects->SetFirstColumnWidth( 200 );

	FillList();

	LoadControlSettings( "shadereditorui/vgui/dialog_ppe_precache_list.res" );

	SetTitle( "Post processing precache", true );

	DoModal();
	SetDeleteSelfOnClose( true );
	MoveToCenterOfScreen();
}

CDialog_PPEPrecache::~CDialog_PPEPrecache()
{
	if ( m_hEffectBrowser.Get() )
		m_hEffectBrowser.Get()->MarkForDeletion();
}


void CDialog_PPEPrecache::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( "add_effect", cmd ) )
	{
		OpenFiledialog();
	}
	else if ( Q_stristr( cmd, "del_effect_" ) == cmd )
	{
		const char *pszIdStart = cmd + 11;
		const int index = atoi( pszIdStart );

		Assert( index >= 0 && index < GetPPCache()->GetNumPostProcessingEffects() );

		GetPPCache()->DeletePostProcessingEffect( index );

		FillList();
	}
	else if ( Q_stristr( cmd, "movedown_effect_" ) == cmd )
	{
		const char *pszIdStart = cmd + 16;
		const int index = atoi( pszIdStart );

		Assert( index >= 0 && index < GetPPCache()->GetNumPostProcessingEffects() );

		GetPPCache()->MoveEffectAlongList( index, false );

		FillList();
	}
	else if ( Q_stristr( cmd, "moveup_effect_" ) == cmd )
	{
		const char *pszIdStart = cmd + 14;
		const int index = atoi( pszIdStart );

		Assert( index >= 0 && index < GetPPCache()->GetNumPostProcessingEffects() );

		GetPPCache()->MoveEffectAlongList( index, true );

		FillList();
	}
	else
	{
		GetPPCache()->SavePrecacheFile();

		BaseClass::OnCommand( cmd );
	}
}

void CDialog_PPEPrecache::FillList()
{
	ScrollBar *pScroll = m_pList_Effects->GetScrollbar();
	int scrollValue = pScroll ? pScroll->GetValue() : 0;

	m_pList_Effects->DeleteAllItems();

	GetPPCache()->ClearInvalidEntries();

	HFont fontMarlett = scheme()->GetIScheme( GetScheme() )->GetFont( "Marlett", false );

	for ( int i = 0; i < GetPPCache()->GetNumPostProcessingEffects(); i++ )
	{
		EditorPostProcessingEffect *effect = GetPPCache()->GetPostProcessingEffect( i );

		Label *pL = new Label( m_pList_Effects, "", effect->pszName );
		Button *pDel = new Button( m_pList_Effects, "", "Delete", this, VarArgs("del_effect_%i",i) );
		CheckButton *pCheck = new CheckButton( m_pList_Effects, effect->pszName, "" );

		pCheck->AddActionSignalTarget( this );
		pCheck->SetSelected( effect->bStartEnabled );

		Button *pDown = new Button( m_pList_Effects, "", "u", this, VarArgs("movedown_effect_%i",i) );
		Button *pUp = new Button( m_pList_Effects, "", "t", this, VarArgs("moveup_effect_%i",i) );

		pDown->SetFont( fontMarlett );
		pUp->SetFont( fontMarlett );

		pDel->SetContentAlignment( Label::a_center );
		pDown->SetContentAlignment( Label::a_center );
		pUp->SetContentAlignment( Label::a_center );

		m_pList_Effects->AddItem( NULL, pCheck );
		m_pList_Effects->AddItem( NULL, pDown );
		m_pList_Effects->AddItem( NULL, pUp );
		m_pList_Effects->AddItem( pL, pDel );
	}

	if ( pScroll )
		pScroll->SetValue( scrollValue );
}

void CDialog_PPEPrecache::CheckButtonChecked( KeyValues *pKV )
{
	bool bChecked = pKV->GetInt( "state" ) != 0;
	Panel *p = (Panel*)pKV->GetPtr( "panel" );
	Assert( p );

	int index = GetPPCache()->FindPostProcessingEffect( p->GetName() );
	Assert( index >= 0 && index < GetPPCache()->GetNumPostProcessingEffects() );

	GetPPCache()->GetPostProcessingEffect( index )->bStartEnabled = bChecked;
	GetPPCache()->GetPostProcessingEffect( index )->bIsEnabled = bChecked;

	if ( pEditorRoot->GetSafeFlowgraph()->GetFlowgraphType() == CNodeView::FLOWGRAPH_POSTPROC )
		pEditorRoot->GetSafeFlowgraph()->UpdatePPECache();
}

void CDialog_PPEPrecache::OnFileSelected( KeyValues *pKV )
{
	const char *pathIn = pKV->GetString( "fullpath" );
	if ( Q_strlen( pathIn ) <= 1 )
		return;

	char tmp[MAX_PATH*4];
	if ( !g_pFullFileSystem->FullPathToRelativePath( pathIn, tmp, sizeof( tmp ) ) )
		Q_snprintf( tmp, sizeof(tmp), "%s", pathIn );

	CPostProcessingCache::PPE_Error error = GetPPCache()->LoadPostProcessingEffect( tmp, false );

	if ( error != CPostProcessingCache::PPE_OKAY )
	{
		PromptSimple *prompt = new PromptSimple( this, "Error" );
		prompt->MoveToCenterOfScreen();
		switch ( error )
		{
		case CPostProcessingCache::PPE_INVALID_INPUT:
				prompt->SetText( "Unable to load file." );
			break;
		case CPostProcessingCache::PPE_INVALID_GRAPH_TYPE:
				prompt->SetText( "This file is not a post processing graph!" );
			break;
		case CPostProcessingCache::PPE_GRAPH_NOT_VALID:
				prompt->SetText( "This graph is not valid, open the file and fix it!" );
			break;
		}
		prompt->AddButton( "Ok" );
	}
	else if ( pEditorRoot->GetSafeFlowgraph()->GetFlowgraphType() == CNodeView::FLOWGRAPH_POSTPROC )
		pEditorRoot->GetSafeFlowgraph()->UpdatePPECache();

	FillList();
}

void CDialog_PPEPrecache::OpenFiledialog()
{
	if ( m_hEffectBrowser.Get() )
		m_hEffectBrowser.Get()->MarkForDeletion();

	m_hEffectBrowser = new FileOpenDialog( this, "Select post processing effect",
		FOD_OPEN, NULL );

	if ( m_hEffectBrowser.Get() )
	{
		m_hEffectBrowser->SetStartDirectoryContext( GetFODPathContext( FODPC_CANVAS ), GetCanvasDirectory() );
		m_hEffectBrowser->AddFilter( "*.txt", "Canvas", true );
		m_hEffectBrowser->DoModal( true );
	}
}
