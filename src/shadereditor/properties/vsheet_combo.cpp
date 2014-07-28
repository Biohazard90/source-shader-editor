
#include "cbase.h"
#include "vSheets.h"


CSheet_Combo::CSheet_Combo(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_Condition = new ComboBox( this, "cbox_condition", 6, false );
	for ( int i = 0; i < 5; i++ )
		m_pCBox_Condition->AddItem( GetConditionCodeString( i ), NULL );

	m_pText_ComboName = new TextEntry( this, "text_cname" );
	m_pText_ComboVar = new TextEntry( this, "text_cvar" );
	m_pCheck_Static = new CheckButton( this, "check_static", "" );

	LoadControlSettings( "shadereditorui/vgui/sheet_combo.res" );
}
CSheet_Combo::~CSheet_Combo()
{
}

void CSheet_Combo::OnResetData()
{
	m_pCBox_Condition->ActivateItem( clamp( pData->GetInt( "i_combo_condition" ), 0, 4 ) );
	m_pText_ComboVar->SetText( VarArgs( "%i", pData->GetInt( "i_combo_value" ) ) );
	m_pText_ComboName->SetText( pData->GetString( "sz_cname" ) );
	m_pCheck_Static->SetSelected( !!pData->GetInt( "i_combo_static" ) );
}
void CSheet_Combo::OnApplyChanges()
{
	char tmp[MAX_PATH];
	char tmp2[MAX_PATH];
	m_pText_ComboName->GetText( tmp, MAX_PATH );
	::CleanupString( tmp, tmp2, MAX_PATH );
	pData->SetString( "sz_cname", tmp2 );

	m_pText_ComboVar->GetText( tmp, MAX_PATH );
	pData->SetInt( "i_combo_value", atoi( tmp ) );

	pData->SetInt( "i_combo_condition", m_pCBox_Condition->GetActiveItem() );
	pData->SetInt( "i_combo_static", m_pCheck_Static->IsSelected() ? 1 : 0 );
}
