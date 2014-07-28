
#include "cbase.h"
#include "editorCommon.h"


CSheet_PP_Mat::CSheet_PP_Mat(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_szMatPath[0] = '\0';
	m_pVMTBuffer = new KeyValues("vmtbuffer");

	m_bPerformedInjection = false;

	m_pCheck_Inline = new CheckButton( this, "check_inline", "Inline material" );

	m_pLoadMaterial = new Button( this, "button_load", "Load material", this, "mat_load" );
	m_pSaveMaterial = new Button( this, "button_save", "Save material", this, "mat_save" );
	m_pLabel_MatPath = new Label( this, "label_matpath", "-" );

	LoadControlSettings( "shadereditorui/vgui/sheet_pp_mat.res" );
}

CSheet_PP_Mat::~CSheet_PP_Mat()
{
	m_pVMTBuffer->deleteThis();
}

void CSheet_PP_Mat::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( "mat_load", cmd ) )
	{
		OpenFiledialog( false );
	}
	else if ( !Q_stricmp( "mat_save", cmd ) )
	{
		OpenFiledialog( true );
	}
	else if ( !Q_stricmp( "edit_vmt", cmd ) )
	{
		char *pszSafe = CKVPacker::KVUnpack( m_pVMTBuffer, "szVMTString" );
		const char *pEditorVmt = CKVPacker::ConvertKVSafeString( pszSafe, false );
		delete [] pszSafe;

		CCode_Editor *pCEdit = new CCode_Editor( this, "ceditinstance", CSmartText::CODEEDITMODE_VMT );
		pCEdit->AddActionSignalTarget( this );
		pCEdit->MoveToFront();

		const char *pszEditorInput = ( pEditorVmt && Q_strlen(pEditorVmt) > 0 ) ? pEditorVmt : DEFAULT_VMT_CONTENT;

		pCEdit->InitVMT( pszEditorInput );

		delete [] pEditorVmt;
	}
	else
		BaseClass::OnCommand( cmd );
}

void CSheet_PP_Mat::OnCodeUpdate( KeyValues *pKV )
{
	char *pUnsafe = CKVPacker::KVUnpack( pKV, "szVMTString" );
	char *pSafeVmt = CKVPacker::ConvertKVSafeString( pUnsafe, true );
	delete [] pUnsafe;
	CKVPacker::KVPack( pSafeVmt, "szVMTString", m_pVMTBuffer );
	//m_pVMTBuffer->SetString( "szVMTString", pSafeVmt );
	delete [] pSafeVmt;

	const bool bDoInjection = pKV->GetInt( "doinject" ) != 0;

	m_bPerformedInjection = m_bPerformedInjection || bDoInjection;

	if ( bDoInjection )
	{
		KeyValues *pInject = pData->MakeCopy();
		WriteDataToKV( pInject );

		n->RestoreFromKeyValues_Specific( pInject );
		pView->MakeSolversDirty();

		pInject->deleteThis();
	}
}

void CSheet_PP_Mat::OnResetData()
{
	const bool bInline = !!pData->GetInt( "iInline" );
	m_pCheck_Inline->SetSelected( bInline );

	CKVPacker::KVCopyPacked( pData, m_pVMTBuffer, "szVMTString" );
	//m_pVMTBuffer->SetString( "szVMTString", pData->GetString("szVMTString"));

	if ( !bInline )
	{
		Q_snprintf( m_szMatPath, sizeof(m_szMatPath), "%s", pData->GetString("szVMTPath") );
		m_pLabel_MatPath->SetText( m_szMatPath );

		DoLoad();
	}
}

void CSheet_PP_Mat::OnApplyChanges()
{
	WriteDataToKV( pData );
}

void CSheet_PP_Mat::OnRestoreNode()
{
	if ( !pData->GetInt( "iInline" ) )
	{
		DoSave();
	}
}

void CSheet_PP_Mat::WriteDataToKV( KeyValues *pKV )
{
	const bool bInline = m_pCheck_Inline->IsSelected() || Q_strlen( m_szMatPath ) < 1;
	pKV->SetInt( "iInline", bInline ? 1 : 0 );

	if ( bInline )
	{
		pKV->SetString( "szVMTPath", "" );
		CKVPacker::KVCopyPacked( m_pVMTBuffer, pKV, "szVMTString" );
		//pKV->SetString( "szVMTString", m_pVMTBuffer->GetString("szVMTString") );
	}
	else
	{
		pKV->SetString( "szVMTString", "" );
		pKV->SetString( "szVMTPath", m_szMatPath );
	}
}

void CSheet_PP_Mat::CheckButtonChecked( KeyValues *pKV )
{
	const bool bChecked = !!pKV->GetInt( "state" );

	m_pLoadMaterial->SetEnabled( !bChecked );
	m_pSaveMaterial->SetEnabled( !bChecked );
	m_pLabel_MatPath->SetVisible( !bChecked );
}

void CSheet_PP_Mat::OnFileSelected( KeyValues *pKV )
{
	KeyValues *pContext = pKV->FindKey( "FileOpenContext" );
	if ( !pContext )
		return;

	const char *__c = pContext->GetString( "context" );
	bool bSaving = true;
	if ( !Q_stricmp( __c, "openc" ) )
		bSaving = false;
	
	const char *pathIn = pKV->GetString( "fullpath" );
	if ( Q_strlen( pathIn ) <= 1 )
		return;

	if ( !g_pFullFileSystem->FullPathToRelativePath( pathIn, m_szMatPath, sizeof(m_szMatPath) ) )
	{
		Q_snprintf( m_szMatPath, sizeof(m_szMatPath), "%s", pathIn );

		vgui::PromptSimple *prompt = new vgui::PromptSimple( this, "ERROR" );
		prompt->MoveToCenterOfScreen();
		prompt->SetText( "The selected file is not located in the game directory!" );
		prompt->AddButton( "Ok" );
		prompt->MakeReadyForUse();
		prompt->InvalidateLayout( true, true );
	}

	m_pLabel_MatPath->SetText( m_szMatPath );

	if ( !bSaving )
	{
		DoLoad();
	}
	else
	{
		DoSave();
	}
}

void CSheet_PP_Mat::DoSave()
{
	if ( Q_strlen(m_szMatPath) < 1 )
		return;

	char *pszVmt_Unpacked = CKVPacker::KVUnpack( m_pVMTBuffer, "szVMTString" );
	const char *pszVmt = pszVmt_Unpacked;
	//const char *pszVmt = m_pVMTBuffer->GetString( "szVMTString" );
	char *pConverted = NULL;

	if ( Q_strlen( pszVmt ) > 0 )
		pszVmt = pConverted = CKVPacker::ConvertKVSafeString( pszVmt, false );
	else
		pszVmt = DEFAULT_VMT_CONTENT;

	CUtlBuffer buf;
	buf.EnsureCapacity( Q_strlen( pszVmt ) + 1 );
	buf.Put( pszVmt, Q_strlen( pszVmt ) );
	g_pFullFileSystem->WriteFile( m_szMatPath, NULL, buf );
	buf.Clear();

	delete [] pszVmt_Unpacked;
	delete [] pConverted;
}

void CSheet_PP_Mat::DoLoad()
{
	CUtlBuffer buf;
	if ( g_pFullFileSystem->ReadFile( m_szMatPath, NULL, buf ) ) // && buf.TellPut() > 0 )
	{
		int size = buf.TellPut();
		char *pszString = new char[ size + 1 ];
		buf.SeekGet( CUtlBuffer::SEEK_HEAD, 0 );
		buf.Get( pszString, size );
		pszString[size] = '\0';

		char *pszSafeString = CKVPacker::ConvertKVSafeString( pszString, true );
		CKVPacker::KVPack( pszSafeString, "szVMTString", m_pVMTBuffer );
		//m_pVMTBuffer->SetString( "szVMTString", pszSafeString );

		delete [] pszSafeString;
		delete [] pszString;
	}
	buf.Clear();
}

void CSheet_PP_Mat::OpenFiledialog( bool bSave )
{
	if ( m_hMaterialBrowser.Get() )
		m_hMaterialBrowser.Get()->MarkForDeletion();

	m_hMaterialBrowser = new FileOpenDialog( this,
		bSave ? "Save material" : "Load material",
		bSave ? FOD_SAVE : FOD_OPEN,
		new KeyValues("FileOpenContext", "context",
		bSave ? "savec" : "openc" )
		);

	if ( m_hMaterialBrowser.Get() )
	{
		m_hMaterialBrowser->SetStartDirectoryContext( GetFODPathContext( FODPC_VMT ), VarArgs( "%s/materials", GetGamePath() ) );
		m_hMaterialBrowser->AddFilter( "*.vmt", "Valve material file", true );
		m_hMaterialBrowser->DoModal( true );
	}
}