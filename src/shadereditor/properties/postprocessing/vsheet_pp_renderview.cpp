
#include "cbase.h"
#include "vSheets.h"

CSheet_PP_RenderView::CSheet_PP_RenderView(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pOptionsList = new PanelListPanel( this, "list_options" );
	m_pCBox_VrNames = new ComboBox( this, "cbox_vrnames", 20, false );
	m_pCheck_Scene = new CheckButton( this, "check_scenedraw", "Render during main scene" );

	for ( int i = 0; i < GetPPCache()->GetNumVrCallbacks(); i++ )
		m_pCBox_VrNames->AddItem( GetPPCache()->GetVrCallback(i)->GetName(), NULL );

	LoadControlSettings( "shadereditorui/vgui/sheet_pp_viewrender.res" );

	m_pOptionsList->SetNumColumns( 1 );
	m_pOptionsList->SetVerticalBufferPixels( 0 );
}

CSheet_PP_RenderView::~CSheet_PP_RenderView()
{
}

void CSheet_PP_RenderView::OnResetData()
{
	const char *pszName = pData->GetString( "szVRCName" );

	bool bReset = true;
	for ( int i = 0; i < GetPPCache()->GetNumVrCallbacks(); i++ )
	{
		if ( !Q_stricmp( GetPPCache()->GetVrCallback(i)->GetName(), pszName ) )
		{
			m_pCBox_VrNames->SetText( GetPPCache()->GetVrCallback(i)->GetName() );
			FillOptions();
			bReset = false;
			break;
		}
	}

	if ( bReset )
		m_pCBox_VrNames->ActivateItem( 0 );
	else
	{
		for ( int i = 0; i < hOptions_Bool.Count(); i++ )
			hOptions_Bool[i]->SetSelected( !!pData->GetInt( VarArgs( "options_bool_%i", i ) ) );

		for ( int i = 0; i < hOptions_Int.Count(); i++ )
			hOptions_Int[i]->SetText( VarArgs( "%i", pData->GetInt( VarArgs( "options_int_%i", i ) ) ) );

		for ( int i = 0; i < hOptions_Float.Count(); i++ )
			hOptions_Float[i]->SetText( VarArgs( "%f", pData->GetFloat( VarArgs( "options_float_%i", i ) ) ) );

		for ( int i = 0; i < hOptions_String.Count(); i++ )
			hOptions_String[i]->SetText( VarArgs( "%s", pData->GetString( VarArgs( "options_string_%i", i ) ) ) );
	}

	m_pCheck_Scene->SetSelected( !!pData->GetInt( "iSceneDraw" ) );
}

void CSheet_PP_RenderView::OnApplyChanges()
{
	char tmp[MAX_PATH*4];
	m_pCBox_VrNames->GetText( tmp, sizeof(tmp) );
	pData->SetString( "szVRCName", tmp );

	pData->SetInt( "options_bool_amt", hOptions_Bool.Count() );
	for ( int i = 0; i < hOptions_Bool.Count(); i++ )
		pData->SetInt( VarArgs( "options_bool_%i", i ), hOptions_Bool[i]->IsSelected() ? 1 : 0 );

	pData->SetInt( "options_int_amt", hOptions_Int.Count() );
	for ( int i = 0; i < hOptions_Int.Count(); i++ )
	{
		hOptions_Int[i]->GetText( tmp, sizeof( tmp ) );
		pData->SetInt( VarArgs( "options_int_%i", i ), atoi( tmp ) );
	}

	pData->SetInt( "options_float_amt", hOptions_Float.Count() );
	for ( int i = 0; i < hOptions_Float.Count(); i++ )
	{
		hOptions_Float[i]->GetText( tmp, sizeof( tmp ) );
		pData->SetFloat( VarArgs( "options_float_%i", i ), atof( tmp ) );
	}

	pData->SetInt( "options_string_amt", hOptions_String.Count() );
	for ( int i = 0; i < hOptions_String.Count(); i++ )
	{
		hOptions_String[i]->GetText( tmp, sizeof( tmp ) );
		pData->SetString( VarArgs( "options_string_%i", i ), tmp );
	}

	pData->SetInt( "iSceneDraw", m_pCheck_Scene->IsSelected() ? 1 : 0 );
}

void CSheet_PP_RenderView::OnTextChanged( KeyValues *pKV )
{
	Panel *p = (Panel*)pKV->GetPtr("panel");

	if ( p == m_pCBox_VrNames )
		FillOptions();
}

void CSheet_PP_RenderView::FillOptions()
{
	m_pOptionsList->DeleteAllItems();

	hOptions_Bool.Purge();
	hOptions_Int.Purge();
	hOptions_Float.Purge();
	hOptions_String.Purge();

	char tmp[MAX_PATH*4];
	m_pCBox_VrNames->GetText( tmp, sizeof(tmp) );
	int vrcIndex = GetPPCache()->FindVrCallback( tmp );

	if ( vrcIndex < 0 )
		return;

	EditorRenderViewCommand_Definition *pVrC = GetPPCache()->GetVrCallback( vrcIndex );

	for ( int i = 0; i < pVrC->GetNumVars( EditorRenderViewCommand_Definition::VAR_BOOL ); i++ )
	{
		Label *pLName = new Label( m_pOptionsList, "", pVrC->GetVarName( EditorRenderViewCommand_Definition::VAR_BOOL, i ) );
		CheckButton *pCheck = new CheckButton( m_pOptionsList, "", "" );
		m_pOptionsList->AddItem( pLName, pCheck );
		hOptions_Bool.AddToTail( pCheck );

		if ( pVrC->defaults.GetNumBool() > 0 )
			pCheck->SetSelected( pVrC->defaults.GetBoolVal( i ) );
	}

	for ( int i = 0; i < pVrC->GetNumVars( EditorRenderViewCommand_Definition::VAR_INT ); i++ )
	{
		Label *pLName = new Label( m_pOptionsList, "", pVrC->GetVarName( EditorRenderViewCommand_Definition::VAR_INT, i ) );
		TextEntry *pText = new TextEntry( m_pOptionsList, "" );
		pText->SetText( "0" );
		m_pOptionsList->AddItem( pLName, pText );
		hOptions_Int.AddToTail( pText );

		if ( pVrC->defaults.GetNumInt() > 0 )
			pText->SetText( VarArgs( "%i", pVrC->defaults.GetIntVal( i ) ) );
	}

	for ( int i = 0; i < pVrC->GetNumVars( EditorRenderViewCommand_Definition::VAR_FLOAT ); i++ )
	{
		Label *pLName = new Label( m_pOptionsList, "", pVrC->GetVarName( EditorRenderViewCommand_Definition::VAR_FLOAT, i ) );
		TextEntry *pText = new TextEntry( m_pOptionsList, "" );
		pText->SetText( "0" );
		m_pOptionsList->AddItem( pLName, pText );
		hOptions_Float.AddToTail( pText );

		if ( pVrC->defaults.GetNumFloat() > 0 )
			pText->SetText( VarArgs( "%f", pVrC->defaults.GetFloatVal( i ) ) );
	}

	for ( int i = 0; i < pVrC->GetNumVars( EditorRenderViewCommand_Definition::VAR_STRING ); i++ )
	{
		Label *pLName = new Label( m_pOptionsList, "", pVrC->GetVarName( EditorRenderViewCommand_Definition::VAR_STRING, i ) );
		TextEntry *pText = new TextEntry( m_pOptionsList, "" );
		pText->SetText( "" );
		m_pOptionsList->AddItem( pLName, pText );
		hOptions_String.AddToTail( pText );

		if ( pVrC->defaults.GetNumString() > 0 )
			pText->SetText( VarArgs( "%s", pVrC->defaults.GetStringVal( i ) ) );
	}
}

void CSheet_PP_RenderView::PerformLayout()
{
	BaseClass::PerformLayout();

	int sx,sy;
	m_pOptionsList->GetSize( sx, sy );

	m_pOptionsList->SetFirstColumnWidth( sx / 2 );
}