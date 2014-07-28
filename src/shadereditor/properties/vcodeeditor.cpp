
#include "cbase.h"
#include "vSheets.h"
#include "vSmartText.h"
#include "vSmartRow.h"
#include "editorcommon.h"

CCode_Editor::CCode_Editor( CSheet_Base *parent, const char *pName, CSmartText::CodeEditMode_t mode ) : BaseClass( parent, pName )
{
	m_iMode = mode;

	Activate();
	SetVisible( true );

	SetPaintBackgroundEnabled(true);
	SetPaintEnabled(true);

	SetSizeable(true);
	SetMoveable(true);
	SetMinimumSize( 500, 400 );

	SetDeleteSelfOnClose( true );

	m_iLastAppliedCodeIndex = 0;

	m_pCodeWindow = new CSmartText( this, "codewindow", mode );
	m_pCodeWindow->AddActionSignalTarget( this );

	/*
	TextEntry *pT = new TextEntry( this, "codewindow" );
	pT->SetMultiline( true );
	pT->SetEditable( true );
	pT->SetCatchEnterKey( true );
	pT->SetVerticalScrollbar( true );
	*/

	LoadControlSettings( "shadereditorui/vgui/code_editor_Window.res" );

	UpdateButtonEnabled( true, true );
	SetCloseButtonVisible(false);

	int w,t;
	surface()->GetScreenSize( w, t );
	w *= 0.75f;
	t *= 0.75f;
	SetSize( max( w, 500 ), max( t, 400 ) );
	
	//SetKeyBoardInputEnabled( false );
	//SetMouseInputEnabled( true );

	//SetTitle( "Editor", true );
	SetTitle( "", false );

	Panel *pButton = FindChildByName( "button_save" );
	if ( pButton )
		pButton->SetKeyBoardInputEnabled( false );

	pButton = FindChildByName( "button_apply" );
	if ( pButton )
		pButton->SetKeyBoardInputEnabled( false );
	pButton = FindChildByName( "button_cancel" );
	if ( pButton )
		pButton->SetKeyBoardInputEnabled( false );

	DoModal();

	int rx, ry, rsx, rsy;
	if ( pEditorRoot->GetCodeEditorBounds( rx, ry, rsx, rsy ) )
		SetBounds( rx, ry, rsx, rsy );
	else
		MoveToCenterOfScreen();
}

CCode_Editor::~CCode_Editor()
{
	int rx, ry, rsx, rsy;
	GetBounds( rx, ry, rsx, rsy );
	pEditorRoot->SetCodeEditorBounds( rx, ry, rsx, rsy );
}

void CCode_Editor::OnKeyCodeTyped( KeyCode code )
{
	if ( code == KEY_B &&
		input()->IsKeyDown( KEY_LSHIFT ) &&
		input()->IsKeyDown( KEY_LCONTROL ) &&
		input()->IsKeyDown( KEY_LALT ) )
	{
		BaseClass::OnKeyCodeTyped( code );
		return;
	}

	if ( IsBuildModeActive() )
		BaseClass::OnKeyCodeTyped( code );
	else
		m_pCodeWindow->OnKeyCodeTyped( code );
}

void CCode_Editor::OnKeyTyped( wchar_t c )
{
	if ( IsBuildModeActive() )
		BaseClass::OnKeyTyped( c );
	else
		m_pCodeWindow->OnKeyTyped( c );
}

void CCode_Editor::InitHlsl( const char *pszFunctionName, CUtlVector< __funcParamSetup* > *input, CUtlVector< __funcParamSetup* > *output,
	const char *pszCodeGlobal, const char *pszCodeBody,
	const int iHlslHierachy, const int iEnvData )
{
	Assert( pszFunctionName && input && output );

	m_pCodeWindow->SetEnvInfo( iEnvData );
	m_pCodeWindow->DoFullParserUpdate( iHlslHierachy );

	char szFunc[MAX_PATH*4];
	BuildUserFunctionName( pszFunctionName, *input, *output, szFunc, sizeof(szFunc) );
	m_pCodeWindow->SetRowDeletable(false);

	const bool bGlobalValid = pszCodeGlobal && Q_strlen(pszCodeGlobal) >= 1;
	if ( bGlobalValid )
	{
		m_pCodeWindow->MoveCursorTo( 0, 0 );
		m_pCodeWindow->ParseInsertText( pszCodeGlobal );
	}

	m_pCodeWindow->InsertRow( szFunc );
	m_pCodeWindow->SetRowLocked(true);

	m_pCodeWindow->InsertRow( "{" );
	m_pCodeWindow->SetRowLocked(true);

	const bool bBodyValid = pszCodeBody && Q_strlen(pszCodeBody) >= 1;
	_smartRow *pBodyFirstLine = m_pCodeWindow->InsertRow( bBodyValid ? NULL : "\t" );
	m_pCodeWindow->SetRowDeletable( false );

	if ( bBodyValid )
	{
		m_pCodeWindow->MoveCursorTo( 0, m_pCodeWindow->FindRow( pBodyFirstLine ) );
		m_pCodeWindow->ParseInsertText( pszCodeBody );
	}

	m_pCodeWindow->InsertRow( "}" );
	m_pCodeWindow->SetRowLocked(true);

	m_pCodeWindow->MoveCursorTo( 1, m_pCodeWindow->FindRow( pBodyFirstLine ) );
}

void CCode_Editor::InitVMT( const char *pszVmt )
{
	Assert( pszVmt && *pszVmt );

	m_pCodeWindow->MoveCursorTo( 0, 0 );
	m_pCodeWindow->ParseInsertText( pszVmt );

	m_pCodeWindow->SetACEnabled( false );
	m_pCodeWindow->SetLiveParseEnabled( false );
	m_pCodeWindow->SetSyntaxHighlightEnabled( false );
}

void CCode_Editor::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( cmd, "save_code" ) )
	{
		ApplyCode( false );

		Close();
	}
	else if ( !Q_stricmp( cmd, "apply_code" ) )
	{
		ApplyCode( true );
	}
	else
		BaseClass::OnCommand( cmd );
}

void CCode_Editor::ApplyCode( bool bDoInject )
{
	if ( m_iMode == CSmartText::CODEEDITMODE_HLSL )
	{
		text_cursor start( 0, 0 );
		text_cursor end( 0, m_pCodeWindow->GetLockedRow( 1 ) - 1 );
		m_pCodeWindow->MoveCursorToEndOfRow( -1, &end );

		text_cursor start_body( 0, m_pCodeWindow->GetLockedRow( 2 ) + 1 );
		text_cursor end_body( 0, m_pCodeWindow->GetLockedRow( 3 ) - 1 );
		m_pCodeWindow->MoveCursorToEndOfRow( -1, &end_body );

		m_pCodeWindow->ClearDynamicSpacing();

		char *pGlobal = ReadSnippet( start, end );
		char *pBody = ReadSnippet( start_body, end_body );

		KeyValues *pCodeMsg = new KeyValues("CodeUpdate");
		if ( pGlobal && Q_strlen( pGlobal ) )
			CKVPacker::KVPack( pGlobal, "szcode_global", pCodeMsg );
			//pCodeMsg->SetString( "szcode_global", pGlobal );
		if ( pBody && Q_strlen( pBody ) )
			CKVPacker::KVPack( pBody, "szcode_body", pCodeMsg );
			//pCodeMsg->SetString( "szcode_body", pBody );
		pCodeMsg->SetInt( "doinject", bDoInject ? 1 : 0 );
		PostActionSignal( pCodeMsg );

		delete [] pGlobal;
		delete [] pBody;
	}
	else if ( m_iMode == CSmartText::CODEEDITMODE_VMT )
	{
		text_cursor start( 0, 0 );
		text_cursor end( -1, -1 );
		m_pCodeWindow->MoveCursorToEndOfText( &end );

		m_pCodeWindow->ClearDynamicSpacing();

		char *pVMT = ReadSnippet( start, end );

		KeyValues *pVMTMsg = new KeyValues("CodeUpdate");
		if ( pVMT )
			CKVPacker::KVPack( pVMT, "szVMTString", pVMTMsg );
			//pVMTMsg->SetString( "szVMTString", pVMT );
		pVMTMsg->SetInt( "doinject", bDoInject ? 1 : 0 );
		PostActionSignal( pVMTMsg );

		delete [] pVMT;
	}
	else
		Assert(0);

	m_iLastAppliedCodeIndex = m_pCodeWindow->GetCurrentHistoryIndex();
	UpdateButtonEnabled( true, false );
}

void CCode_Editor::OnCodeChanged()
{
	HHISTORYIDX curIdx = m_pCodeWindow->GetCurrentHistoryIndex();

	UpdateButtonEnabled( true, curIdx != m_iLastAppliedCodeIndex );
}

void CCode_Editor::UpdateButtonEnabled( bool bShowSave, bool bShowApply )
{
	Panel *pButton = FindChildByName( "button_save" );
	if ( pButton )
		pButton->SetEnabled( bShowSave );

	pButton = FindChildByName( "button_apply" );
	if ( pButton )
		pButton->SetEnabled( bShowApply );
}

char *CCode_Editor::ReadSnippet( text_cursor c0, text_cursor c1, bool bAddCR )
{
	if ( c1 < c0 )
		return NULL;

	wchar_t *pW;
	m_pCodeWindow->Selection_Copy( &c0, &c1, &pW, bAddCR );

	if ( !pW )
		return NULL;

	bool bValid = false;
	wchar_t *pFindChar = pW;

	while ( *pFindChar && !bValid )
	{
		if ( *pFindChar != L' ' &&
			*pFindChar != L'\t' &&
			*pFindChar != L'\r' &&
			*pFindChar != L'\n' )
			bValid = true;
		pFindChar++;
	}

	char *pC = NULL;

	if ( bValid )
	{
		int len = Q_wcslen( pW ) + 1;
		pC = new char[ len ];
		Q_UnicodeToUTF8( pW, pC, len );
	}

	delete [] pW;
	return pC;
}

#if DEBUG
CON_COMMAND( memTest, "" )
{
	CUtlVector< unsigned char* >hMem;
	for ( int i = 0; i < 9999999; i++ )
	{
		//int len = 1; //RandomInt( 0, 2 ) ? RandomInt( 1, 5 ) : RandomInt( 1, 99 );

		unsigned char *_array = new unsigned char;

		//for ( int a = 0; a < len; a++ )
			*_array = 255; //RandomInt( 0, 255 );

		hMem.AddToTail( _array );
	}

	for ( int i = 0; i < hMem.Count(); i++ )
		delete hMem[i];

	hMem.Purge();
}
#endif