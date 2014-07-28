
#include "cbase.h"
#include "editorCommon.h"

CShaderPrecache::CShaderPrecache( vgui::Panel *parent, CNodeView *nodeview ) : BaseClass( parent, nodeview, "shaderpreview" )
{
	m_pPanelList_Shader = new PanelListPanel( this, "shaderlist" );
	m_pPanelList_Shader->SetFirstColumnWidth( 300 );

	m_pCheck_ReloadCache = new CheckButton( this, "check_reloadcache", "Reload cache on close" );

	LoadControlSettings("shadereditorui/vgui/shadereditor_shaderprecache.res");

	SetTitle("Shader precache",true);

	SetAutoDelete( true );
	SetSizeable(false);

	int sx, sy;
	surface()->GetScreenSize( sx, sy );
	SetSize( 480, sy * 0.8f );

	DoModal();

	LoadList();
}

CShaderPrecache::~CShaderPrecache()
{
	m_pPanelList.Purge();
}

void CShaderPrecache::Activate()
{
	BaseClass::Activate();
}

void CShaderPrecache::Paint()
{
	BaseClass::Paint();
}

void CShaderPrecache::OnCommand(const char* pcCommand)
{
	if ( !Q_stricmp( pcCommand, "addshader" ) )
	{
		OpenFileBrowser();
		return;
	}
	else if ( !Q_stricmp( pcCommand, "close" ) )
	{
		SaveList();

		pEditorRoot->m_bAutoPrecacheUpdate = m_pCheck_ReloadCache->IsSelected();

		if ( pEditorRoot->ShouldAutoUpdatePrecache() )
		{
			LoadGameShaders();
			QuickRefreshEditorShader();

			GetPPCache()->RefreshAllPPEMaterials();
		}
	}
	BaseClass::OnCommand(pcCommand);
}

void CShaderPrecache::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CShaderPrecache::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pCheck_ReloadCache->SetSelected( pEditorRoot->ShouldAutoUpdatePrecache() );
}

void CShaderPrecache::LoadList()
{
	KeyValues *pKV = df_LoadDump_List();
	KeyValues *pKey = pKV->GetFirstValue();
	while ( pKey )
	{
		AddEntry( pKey->GetString() );
		pKey = pKey->GetNextValue();
	}

	pKV->deleteThis();
}
void CShaderPrecache::AddEntry( const char *name )
{
	char szFullpath[MAX_PATH];
	Q_snprintf( szFullpath, MAX_PATH, "%s\\%s.dump", ::GetDumpDirectory(), name );
	Q_FixSlashes( szFullpath );

	if ( !g_pFullFileSystem->FileExists( szFullpath, "MOD" ) )
		return;

	for ( int i = 0; i < m_pPanelList.Count(); i++ )
	{
		char tmp[MAX_PATH];
		m_pPanelList[i].L->GetText( tmp, MAX_PATH );
		if ( !Q_stricmp( tmp, name ) )
			return;
	}

	Label *pL = new Label( m_pPanelList_Shader, "", name );
	Button *pB = new Button( m_pPanelList_Shader, "", "Remove" );
	pB->AddActionSignalTarget( this );
	pB->SetContentAlignment( Label::a_center );

	m_pPanelList.AddToTail();
	m_pPanelList.Tail().L = pL;
	m_pPanelList.Tail().B = pB;

	SortAllEntries();
}
int PEntrySort( PListEntry_t const *p1, PListEntry_t const *p2 )
{
	char sz1[ MAX_PATH ];
	char sz2[ MAX_PATH ];
	( *p1 ).L->GetText( sz1, sizeof( sz1 ) );
	( *p2 ).L->GetText( sz2, sizeof( sz2 ) );
	return Q_stricmp( sz1, sz2 );
}
void CShaderPrecache::SortAllEntries()
{
	m_pPanelList.Sort( PEntrySort );
	m_pPanelList_Shader->RemoveAll();
	for ( int i = 0; i < m_pPanelList.Count(); i++ )
		m_pPanelList_Shader->AddItem( m_pPanelList[i].L, m_pPanelList[i].B );
	UpdateAllButtonCmds();
}
void CShaderPrecache::SetButtonCmd( Button *b )
{
	Assert( PListEntry_t::HasElement( m_pPanelList, b ) );

	int idx = PListEntry_t::FindEntry( m_pPanelList, b );
	b->SetCommand( new KeyValues( "shaderremove", "index", idx ) );
}
void CShaderPrecache::SaveList()
{
	KeyValues *pKV = new KeyValues( "shaderlist" );

	for ( int i = 0; i < m_pPanelList.Count(); i++ )
	{
		char name[MAX_PATH];
		m_pPanelList[i].L->GetText( name, MAX_PATH );
		pKV->SetString( VarArgs( "shader_%03i", i ), name );
	}

	df_SaveDump_List( pKV );
}
void CShaderPrecache::OnRemovePressed( int index )
{
	m_pPanelList.Remove( index );
	m_pPanelList_Shader->RemoveItem( m_pPanelList_Shader->GetItemIDFromRow( index ) );
	SortAllEntries();
}
void CShaderPrecache::UpdateAllButtonCmds()
{
	Assert( m_pPanelList.Count() == m_pPanelList_Shader->GetItemCount() );

	for ( int i = 0; i < m_pPanelList.Count(); i++ )
		SetButtonCmd( m_pPanelList[i].B );
}


void CShaderPrecache::OpenFileBrowser()
{
	if ( m_hDumpBrowser.Get() )
		m_hDumpBrowser.Get()->MarkForDeletion();

	m_hDumpBrowser = new FileOpenDialog( this,
		"Load shader info",
		FOD_OPEN,
		new KeyValues("FileOpenContext", "context", "open_si" ) );

	if ( m_hDumpBrowser.Get() )
	{
		m_hDumpBrowser->SetStartDirectoryContext( GetFODPathContext( FODPC_DUMPFILE ), ::GetDumpDirectory() );
		m_hDumpBrowser->AddFilter( "*.dump", "ShaderInfo", true );
		m_hDumpBrowser->DoModal();
	}
}
void CShaderPrecache::OnFileSelected( KeyValues *pKV )
{
	const char *pathIn = pKV->GetString( "fullpath" );
	if ( Q_strlen( pathIn ) <= 1 )
		return;

	char dumpname[MAX_PATH];
	Q_FileBase( pathIn, dumpname, MAX_PATH );
	AddEntry( dumpname );
}