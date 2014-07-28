
#include "cbase.h"
#include "vSheets.h"


CSheet_General::CSheet_General(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );
	m_pCheck_Preview = NULL;

	m_pText_Name = new TextEntry(this, "name");
	if ( !pNode->IsNodeCrucial() )
		m_pCheck_Preview = new CheckButton(this, "preview","");

	m_pLabel_Name = new Label( this, "label_name", "NAME" );
	m_pLabel_Info = new Label( this, "label_info", "INFO" );
	m_pLabel_Example = new Label( this, "label_example", "EXAMPLE" );

	LoadControlSettings( "shadereditorui/vgui/sheet_general.res" );

	m_pLabel_Name->MoveToFront();
	m_pLabel_Info->MoveToFront();
	m_pLabel_Example->MoveToFront();

	m_pLabel_Name->SetText( "" );
	m_pLabel_Info->SetText( "" );
	m_pLabel_Example->SetText( "" );

	KeyValues *pNodeHelp = pEditorRoot->GetNodeHelpContainer();
	if ( pNodeHelp )
	{
		KeyValues *pKV = pNodeHelp->FindKey( VarArgs( "%i", pNode->GetNodeType() ) );
		if ( pKV )
		{
			char *pszName = CKVPacker::ConvertKVSafeString( pKV->GetString( "name" ), false );
			char *pszInfo = CKVPacker::ConvertKVSafeString( pKV->GetString( "info" ), false );
			char *pszCode = CKVPacker::ConvertKVSafeString( pKV->GetString( "code" ), false );

			if ( pszName )
				m_pLabel_Name->SetText( pszName );
			if ( pszInfo )
				m_pLabel_Info->SetText( pszInfo );
			if ( pszCode )
				m_pLabel_Example->SetText( VarArgs( "Example: %s", pszCode ) );

			delete [] pszName;
			delete [] pszInfo;
			delete [] pszCode;
		}
	}
}
CSheet_General::~CSheet_General()
{
}

void CSheet_General::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	Panel *p = FindChildByName( "info_bg" );
	if ( p )
		p->SetBorder( pScheme->GetBorder( "ButtonDepressedBorder" ) );
}

void CSheet_General::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pLabel_Name->SetFgColor( Color( 255, 255, 255, 255 ) );
	m_pLabel_Info->SetFgColor( Color( 210, 210, 210, 255 ) );
	m_pLabel_Example->SetFgColor( Color( 200, 210, 250, 255 ) );
}

void CSheet_General::OnResetData()
{
	m_pText_Name->SetText( pData->GetString( "szName" ) );
	if ( m_pCheck_Preview )
		m_pCheck_Preview->SetSelected( ( pData->GetInt( "iPreview" ) != 0 ) );
}
void CSheet_General::OnApplyChanges()
{
	char name[MAX_PATH];
	char nameModified[MAX_PATH];
	m_pText_Name->GetText( name, MAX_PATH );
	//CleanupString( name, nameModified, MAX_PATH, true );
	Q_strcpy( nameModified, name );

	pData->SetString( "szName", nameModified );

	if ( m_pCheck_Preview )
		pData->SetInt( "iPreview", m_pCheck_Preview->IsSelected() ? 1 : 0 );
}
