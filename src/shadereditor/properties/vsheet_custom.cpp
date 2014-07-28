
#include "cbase.h"
#include "vSheets.h"

__customCtrlEntry::__customCtrlEntry()
{
	pName = NULL;
	pType = NULL;
	pDel = NULL;
}

void __customCtrlEntry::Delete( bool killChildren )
{
	if ( killChildren )
	{
		if ( pName != NULL )
		{
			pName->SetParent( (Panel*)NULL );
			pName->MarkForDeletion();
		}
		if ( pType != NULL )
		{
			pType->SetParent( (Panel*)NULL );
			pType->MarkForDeletion();
		}
		if ( pDel != NULL )
		{
			pDel->SetParent( (Panel*)NULL );
			pDel->MarkForDeletion();
		}
	}
	delete this;
}

CSheet_Custom::CSheet_Custom(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	m_pIO_Page = NULL;
	m_szFilePath[0] = '\0';
	m_bPerformedInjection = false;
	m_pKVCodeCache = new KeyValues("");

	m_pEntry_FuncName = new TextEntry( this, "function_name" );
	m_pLabel_Path = new Label( this, "file_path", "-" );

	m_pCheck_Inline = new CheckButton( this, "check_inline", "Save in canvas" );
	m_pCheck_Lighting_PS = new CheckButton( this, "check_lighting_ps", "Add pixel shader lighting" );
	m_pCheck_Lighting_VS = new CheckButton( this, "check_lighting_vs", "Add vertex shader lighting" );

	LoadControlSettings( "shadereditorui/vgui/sheet_custom_code.res" );

	int iType = n->GetHierachyTypeIterateFullyRecursive();
	bool bIsPS = ( iType & HLSLHIERACHY_PS ) != 0;
	bool bIsVS = ( iType & HLSLHIERACHY_VS ) != 0;

	m_pCheck_Lighting_PS->SetEnabled( bIsPS || !bIsVS );
	m_pCheck_Lighting_VS->SetEnabled( bIsVS || !bIsPS );

	m_pCheck_Inline->SetSelected( true );
}

CSheet_Custom::~CSheet_Custom()
{
	m_pKVCodeCache->deleteThis();
}

void CSheet_Custom::SetIOPage( CSheet_Custom_IO *p )
{
	m_pIO_Page = p;
}

bool CSheet_Custom::RequiresReset()
{
	return m_bPerformedInjection;
}

void CSheet_Custom::OnCodeUpdate( KeyValues *pKV )
{
	//const char *pCodeGlobal = pKV->GetString( "szcode_global" );
	//const char *pCodeBody = pKV->GetString( "szcode_body" );
	char *pCodeGlobal = CKVPacker::KVUnpack( pKV, "szcode_global" );
	char *pCodeBody = CKVPacker::KVUnpack( pKV, "szcode_body" );

	const bool bDoInjection = pKV->GetInt( "doinject" ) != 0;

	m_bPerformedInjection = m_bPerformedInjection || bDoInjection;

	char *pszSafeG = CKVPacker::ConvertKVSafeString( pCodeGlobal, true );
	char *pszSafeB = CKVPacker::ConvertKVSafeString( pCodeBody, true );

	//m_pKVCodeCache->SetString( "szcode_global", pszSafeG );
	//m_pKVCodeCache->SetString( "szcode_body", pszSafeB );
	CKVPacker::KVPack( pszSafeG, "szcode_global", m_pKVCodeCache );
	CKVPacker::KVPack( pszSafeB, "szcode_body", m_pKVCodeCache );

	delete [] pCodeGlobal;
	delete [] pCodeBody;

	delete [] pszSafeG;
	delete [] pszSafeB;

	if ( bDoInjection )
	{
		KeyValues *pKV_Injection = pData->MakeCopy();

		WriteDataToKV( pKV_Injection, false );
		m_pIO_Page->WriteDataToKV( pKV_Injection );

		n->RestoreFromKeyValues_Specific( pKV_Injection );
		pView->MakeSolversDirty();

		pKV_Injection->deleteThis();
	}
}

void CSheet_Custom::OnCommand( KeyValues *pKV )
//void CSheet_Custom::OnCommand( const char *cmd )
{
	const char *cmd = pKV->GetString( "command" );

	if ( !Q_stricmp( cmd, "editcode" ) )
	{
		Assert( m_pIO_Page );

		int iType = n->GetHierachyTypeIterateFullyRecursive();

		m_pIO_Page->AllocNodeData();

		CCode_Editor *pCEdit = new CCode_Editor( this, "ceditinstance" );
		pCEdit->AddActionSignalTarget( this );
		pCEdit->MoveToFront();

		char szFunc[MAX_PATH];
		szFunc[0] = 0;
		m_pEntry_FuncName->GetText( szFunc, sizeof( szFunc ) );
		if ( !Q_strlen( szFunc ) )
			BuildDefaultFunctionName(szFunc, sizeof(szFunc), n );

		char *pszUnpackedGlobal = CKVPacker::KVUnpack( m_pKVCodeCache, "szcode_global" );
		char *pszUnpackedBody = CKVPacker::KVUnpack( m_pKVCodeCache, "szcode_body" );

		char *pUnSafeG = CKVPacker::ConvertKVSafeString( pszUnpackedGlobal, false );
		char *pUnSafeB = CKVPacker::ConvertKVSafeString( pszUnpackedBody, false );

		pCEdit->InitHlsl( szFunc, &m_pIO_Page->m_hszVarNames_In, &m_pIO_Page->m_hszVarNames_Out,
			pUnSafeG, pUnSafeB, iType, BuildEnvDataFlags() );

		delete [] pszUnpackedGlobal;
		delete [] pszUnpackedBody;

		delete [] pUnSafeG;
		delete [] pUnSafeB;
	}
	else if ( !Q_stricmp( cmd, "save_code" ) )
	{
		OpenFiledialog( true );
	}
	else if ( !Q_stricmp( cmd, "load_code" ) )
	{
		OpenFiledialog( false );
	}
	else
		BaseClass::OnCommand( cmd );
}

int CSheet_Custom::BuildEnvDataFlags()
{
	int i = 0;
	if ( m_pCheck_Lighting_PS->IsSelected() ) //&& m_pCheck_Lighting_PS->IsEnabled() )
		i |= NODECUSTOM_ADDENV_LIGHTING_PS;
	if ( m_pCheck_Lighting_VS->IsSelected() ) //&& m_pCheck_Lighting_VS->IsEnabled() )
		i |= NODECUSTOM_ADDENV_LIGHTING_VS;
	return i;
}

void CSheet_Custom::OnResetData()
{
	m_pEntry_FuncName->SetText("");

	const char *pszFunc = pData->GetString( "szFunctionName" );
	const char *pWalk = pszFunc;
	while ( pWalk && *pWalk )
	{
		if ( *pWalk != ' ' )
		{
			m_pEntry_FuncName->SetText( pszFunc );
			break;
		}

		pWalk++;
	}
	const bool bInline = pData->GetInt( "iInline", 1 ) != 0;
	m_pCheck_Inline->SetSelected( bInline );

	const char *pszPath = pData->GetString( "szFilePath" );
	if ( !bInline )
		Q_snprintf( m_szFilePath, sizeof( m_szFilePath ), "%s", pszPath );

	CKVPacker::KVCopyPacked( pData, m_pKVCodeCache, "szcode_global" );
	CKVPacker::KVCopyPacked( pData, m_pKVCodeCache, "szcode_body" );

	//m_pKVCodeCache->SetString( "szcode_global", pData->GetString( "szcode_global" ) );
	//m_pKVCodeCache->SetString( "szcode_body", pData->GetString( "szcode_body" ) );

	int iEnvFlags = pData->GetInt( "iEnvFlags" );

	m_pCheck_Lighting_PS->SetSelected( ( iEnvFlags & NODECUSTOM_ADDENV_LIGHTING_PS ) != 0 );
	m_pCheck_Lighting_VS->SetSelected( ( iEnvFlags & NODECUSTOM_ADDENV_LIGHTING_VS ) != 0 );
}

void CSheet_Custom::OnApplyChanges()
{
	WriteDataToKV( pData );
}

void CSheet_Custom::WriteDataToKV( KeyValues *pKV, bool bCheckFilePath )
{
	char szFunc[MAX_PATH];
	szFunc[0] = 0;
	m_pEntry_FuncName->GetText( szFunc, sizeof( szFunc ) );

	bool bFileValid = Q_strlen(m_szFilePath) && ( !bCheckFilePath || g_pFullFileSystem->FileExists( m_szFilePath ) );

	if ( !bFileValid )
		m_szFilePath[0] = '\0';

	pKV->SetString( "szFunctionName", szFunc );
	pKV->SetString( "szFilePath", m_szFilePath );
	pKV->SetInt( "iInline", ( m_pCheck_Inline->IsSelected() || !bFileValid ) ? 1 : 0 );

	CKVPacker::KVCopyPacked( m_pKVCodeCache, pKV, "szcode_global" );
	CKVPacker::KVCopyPacked( m_pKVCodeCache, pKV, "szcode_body" );
	//pKV->SetString( "szcode_global", m_pKVCodeCache->GetString( "szcode_global" ) );
	//pKV->SetString( "szcode_body", m_pKVCodeCache->GetString( "szcode_body" ) );

	int iEnvFlags = BuildEnvDataFlags();

	pKV->SetInt( "iEnvFlags", iEnvFlags );
}

void CSheet_Custom::OnRestoreNode()
{
	if ( !m_pCheck_Inline->IsSelected() && Q_strlen( m_szFilePath ) )
	{
		DoSave();

		pEditorRoot->RefreshNodeInstances( HLSLNODE_UTILITY_CUSTOMCODE, new KeyValues( "info", "filepath", m_szFilePath ) );
	}
}


void CSheet_Custom::CheckButtonChecked( KeyValues *pKV )
{
	bool bChecked = pKV->GetInt( "state" ) != 0;
	Panel *p = (Panel*)pKV->GetPtr( "panel" );

	Assert( p );

	if ( p == m_pCheck_Inline )
	{
		Panel *p = FindChildByName( "button_save" );
		if ( p )
			p->SetEnabled( !bChecked );
		p = FindChildByName( "button_load" );
		if ( p )
			p->SetEnabled( !bChecked );

		m_pLabel_Path->SetText( bChecked ? "-" : m_szFilePath );
	}
}

void CSheet_Custom::OnFileSelected( KeyValues *pKV )
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

	if ( !g_pFullFileSystem->FullPathToRelativePath( pathIn, m_szFilePath, sizeof(m_szFilePath) ) )
	{
		Q_snprintf( m_szFilePath, sizeof(m_szFilePath), "%s", pathIn );

		vgui::PromptSimple *prompt = new vgui::PromptSimple( this, "ERROR" );
		prompt->MoveToCenterOfScreen();
		prompt->SetText( "The selected file is not located in the game directory!" );
		prompt->AddButton( "Ok" );
		prompt->MakeReadyForUse();
		prompt->InvalidateLayout( true, true );
	}

	m_pLabel_Path->SetText( m_szFilePath );

	if ( bSaving )
	{
		DoSave();
	}
	else
	{
		DoLoad();
	}
}

void CSheet_Custom::DoSave()
{
	KeyValues *pCodeFile = new KeyValues("user_func");

	WriteDataToKV( pCodeFile, false );
	m_pIO_Page->WriteDataToKV( pCodeFile );

	pCodeFile->SaveToFile( g_pFullFileSystem, m_szFilePath, "MOD" );
	pCodeFile->deleteThis();

	//m_bPerformedInjection = false;
}

void CSheet_Custom::DoLoad()
{
	KeyValues *pCodeFile = new KeyValues("");

	if ( pCodeFile->LoadFromFile( g_pFullFileSystem, m_szFilePath, "MOD" ) )
	{
		m_pIO_Page->ClearIOs( pData );
		//pData->SetString( "szcode_global", "" );
		//pData->SetString( "szcode_body", "" );
		CKVPacker::KVClearPacked( "szcode_global", pData );
		CKVPacker::KVClearPacked( "szcode_body", pData );

		for ( KeyValues *pKey = pCodeFile->GetFirstValue(); pKey; pKey = pKey->GetNextValue() )
		{
			const char *pszName = pKey->GetName();
			switch ( pKey->GetDataType() )
			{
			case KeyValues::TYPE_STRING:
				pData->SetString( pszName, pKey->GetString() );
				break;
			case KeyValues::TYPE_INT:
				pData->SetInt( pszName, pKey->GetInt() );
				break;
			}
		}
	}

	pCodeFile->deleteThis();

	m_pIO_Page->PurgeNameVecs();

	OnResetData();
	m_pIO_Page->OnResetData();
}

void CSheet_Custom::OpenFiledialog( bool bSave )
{
	if ( m_hCodeBrowser.Get() )
		m_hCodeBrowser.Get()->MarkForDeletion();

	m_hCodeBrowser = new FileOpenDialog( this,
		bSave ? "Save code" : "Load code",
		bSave ? FOD_SAVE : FOD_OPEN,
		new KeyValues("FileOpenContext", "context",
		bSave ? "savec" : "openc" )
		);

	if ( m_hCodeBrowser.Get() )
	{
		m_hCodeBrowser->SetStartDirectoryContext( GetFODPathContext( FODPC_USERFUNCTION ), GetUserFunctionDirectory() );
		m_hCodeBrowser->AddFilter( "*.ufunc", "User function", true );
		m_hCodeBrowser->DoModal( true );
	}
}






CSheet_Custom_IO::CSheet_Custom_IO(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	m_pList_Jacks = new PanelListPanel( this, "list_data_parent" );
	m_pList_Jacks->SetNumColumns( 1 );
	m_pList_Jacks->SetFirstColumnWidth( 0 );

	AllocList();

	LoadControlSettings( "shadereditorui/vgui/sheet_custom_code_io.res" );
}
CSheet_Custom_IO::~CSheet_Custom_IO()
{
	m_hCtrl_In.PurgeAndDeleteElements();
	m_hCtrl_Out.PurgeAndDeleteElements();

	PurgeNameVecs();
}
void CSheet_Custom_IO::PurgeNameVecs()
{
	m_hszVarNames_In.PurgeAndDeleteElements();
	m_hszVarNames_Out.PurgeAndDeleteElements();
}

__customCtrlEntry *CSheet_Custom_IO::AllocEntry( int idx, bool bOutput, __funcParamSetup *data, Panel **p )
{
	PanelListPanel *pRow = new PanelListPanel( NULL, "" );
	pRow->SetInnerPanel( true );
	pRow->SetNumColumns( 3 );
	pRow->SetFirstColumnWidth( 15 );
	pRow->SetVerticalBufferPixels( 0 );

	Label *pName = new Label( pRow, "", VarArgs( "%i:", idx ) );

	TextEntry *pIn = new TextEntry( pRow, "" );
	pIn->SetMaximumCharCount( 32 );
	pIn->SetText( data->pszName );

	ComboBox *pCBoxType = new ComboBox( pRow, "", 10, false );
	FillComboBox( pCBoxType );
	pCBoxType->ActivateItem( GetVarFlagsVarValue( data->iFlag ) );

	Button *pButRemove = new Button( pRow, "", "Delete" );
	KeyValues *pCmd = new KeyValues( "Command" );
	pCmd->SetString( "command", bOutput ? "removeoutput" : "removeinput" );
	pCmd->SetInt( "entryidx", idx );
	pButRemove->SetCommand( pCmd );
	pButRemove->AddActionSignalTarget( this );

	pRow->AddItem( pName, pIn );
	pRow->AddItem( NULL, pCBoxType );
	pRow->AddItem( NULL, pButRemove );

	pRow->SetPaintBackgroundEnabled( false );
	pRow->SetPaintBorderEnabled( false );
	int w, t;
	pRow->GetSize( w, t );
	pRow->SetSize( w, 26 );
	Panel *pScrollBar = pRow->FindChildByName( "PanelListPanelVScroll" );
	Assert( pScrollBar != NULL );
	pScrollBar->SetSize( 0, 0 );

	__customCtrlEntry *pEntry = new __customCtrlEntry();
	pEntry->pName = pIn;
	pEntry->pDel = pButRemove;
	pEntry->pType = pCBoxType;

	//pRow->InvalidateLayout(true,true);
	//InvalidateLayout();

	*p = pRow;
	return pEntry;
}
void CSheet_Custom_IO::AllocList()
{
	Assert( m_pList_Jacks != NULL );

	m_pList_Jacks->DeleteAllItems();
	m_hCtrl_In.PurgeAndDeleteElements();
	m_hCtrl_Out.PurgeAndDeleteElements();

	InvalidateLayout( true, true );

	m_pList_Jacks->AddItem( NULL, new Label( NULL, "", "Inputs" ) );

	for ( int i = 0; i < m_hszVarNames_In.Count(); i++ )
	{
		Panel *pRow = NULL;
		m_hCtrl_In.AddToTail( AllocEntry( i, false, m_hszVarNames_In[ i ], &pRow ) );
		m_pList_Jacks->AddItem( NULL, pRow );
		pRow->InvalidateLayout( true, true );
	}

	m_pList_Jacks->AddItem( NULL, new Label( NULL, "", "Outputs" ) );

	for ( int i = 0; i < m_hszVarNames_Out.Count(); i++ )
	{
		Panel *pRow = NULL;
		m_hCtrl_Out.AddToTail( AllocEntry( i, true, m_hszVarNames_Out[ i ], &pRow ) );
		m_pList_Jacks->AddItem( NULL, pRow );
		pRow->InvalidateLayout( true, true );
	}
}

void CSheet_Custom_IO::AllocNodeData()
{
	PurgeNameVecs();

	for ( int i = 0; i < m_hCtrl_In.Count(); i++ )
	{
		__customCtrlEntry *ctrl = m_hCtrl_In[ i ];

		Assert( ctrl->pName != NULL );

		char tmp[33];
		ctrl->pName->GetText( tmp, 33 );

		m_hszVarNames_In.AddToTail(
			CNodeCustom::AllocOutputSetup( false, tmp, ctrl->pType ? GetVarTypeFlag( ctrl->pType->GetActiveItem() ) : HLSLVAR_FLOAT1 )
			);
	}
	for ( int i = 0; i < m_hCtrl_Out.Count(); i++ )
	{
		__customCtrlEntry *ctrl = m_hCtrl_Out[ i ];

		Assert( ctrl->pName != NULL );

		char tmp[33];
		ctrl->pName->GetText( tmp, 33 );

		m_hszVarNames_Out.AddToTail(
			CNodeCustom::AllocOutputSetup( true, tmp, ctrl->pType ? GetVarTypeFlag( ctrl->pType->GetActiveItem() ) : HLSLVAR_FLOAT1 )
			);
	}
}

void CSheet_Custom_IO::FillComboBox( ComboBox *c )
{
	for ( int i = 0; i < NUM_CBOX_VARTYPES_CFULL; i++ )
	{
		KeyValues *data = new KeyValues( VarArgs( "vartype_%i", i ), "vartype", i );
		c->AddItem( GetVarTypeName( i ), data );
		data->deleteThis();
	}
}

void CSheet_Custom_IO::OnCommand( KeyValues *pKV )
//void CSheet_Custom::OnCommand( const char *cmd )
{
	const char *cmd = pKV->GetString( "command" );

	if ( !Q_stricmp( cmd, "addoutput" ) )
	{
		AllocNodeData();
		m_hszVarNames_Out.AddToTail( CNodeCustom::AllocOutputSetup( true, "", HLSLVAR_FLOAT1 ) );
		AllocList();
	}
	else if ( !Q_stricmp( cmd, "addinput" ) )
	{
		AllocNodeData();
		m_hszVarNames_In.AddToTail( CNodeCustom::AllocOutputSetup( false, "", HLSLVAR_FLOAT1 ) );
		AllocList();
	}
	else if ( !Q_stricmp( cmd, "removeinput" ) )
	{
		AllocNodeData();
		int idx = pKV->GetInt( "entryidx", -1 );
		Assert( idx >= 0 && idx < m_hszVarNames_In.Count() );
		delete m_hszVarNames_In[ idx ];
		m_hszVarNames_In.Remove( idx );
		AllocList();
	}
	else if ( !Q_stricmp( cmd, "removeoutput" ) )
	{
		AllocNodeData();
		int idx = pKV->GetInt( "entryidx", -1 );
		Assert( idx >= 0 && idx < m_hszVarNames_Out.Count() );
		delete m_hszVarNames_Out[ idx ];
		m_hszVarNames_Out.Remove( idx );
		AllocList();
	}
	else if ( !Q_stricmp( cmd, "resetio" ) )
	{
		PurgeNameVecs();
		AllocList();
	}
	else
		BaseClass::OnCommand( cmd );
}

void CSheet_Custom_IO::OnResetData()
{
	CNodeCustom::FillJackDataFromKV( pData, m_hszVarNames_In, m_hszVarNames_Out );

	AllocList();
}

void CSheet_Custom_IO::OnApplyChanges()
{
	WriteDataToKV( pData );
}

void CSheet_Custom_IO::WriteDataToKV( KeyValues *pKV )
{
	AllocNodeData();

	CNodeCustom::WriteJackDataFromKV( pKV, m_hszVarNames_In, m_hszVarNames_Out,
						m_hszVarNames_In.Count(), m_hszVarNames_Out.Count() );


	ClearIOs( pKV, m_hszVarNames_In.Count(), m_hszVarNames_Out.Count() );
}

void CSheet_Custom_IO::ClearIOs( KeyValues *pKV, int firstIn, int firstOut )
{
	KeyValues *pSub = NULL;

	int itr = firstIn;
	for (;;)
	{
		pSub = pKV->FindKey( VarArgs( "varType_In_%i", itr ) );
		if ( pSub == NULL )
			break;

		pKV->RemoveSubKey( pSub );
		pSub->deleteThis();
		itr++;
	}
	itr = firstIn;
	for (;;)
	{
		pSub = pKV->FindKey( VarArgs( "varName_In_%i", itr ) );
		if ( pSub == NULL )
			break;

		pKV->RemoveSubKey( pSub );
		pSub->deleteThis();
		itr++;
	}

	itr = firstOut;
	for (;;)
	{
		pSub = pKV->FindKey( VarArgs( "varType_Out_%i", itr ) );
		if ( pSub == NULL )
			break;

		pKV->RemoveSubKey( pSub );
		pSub->deleteThis();
		itr++;
	}
	itr = firstOut;
	for (;;)
	{
		pSub = pKV->FindKey( VarArgs( "varName_Out_%i", itr ) );
		if ( pSub == NULL )
			break;

		pKV->RemoveSubKey( pSub );
		pSub->deleteThis();
		itr++;
	}
}
