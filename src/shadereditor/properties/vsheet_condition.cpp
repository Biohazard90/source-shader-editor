
#include "cbase.h"
#include "vSheets.h"


CSheet_Condition::CSheet_Condition(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_Condition = new ComboBox( this, "cbox_condition", 6, false );
	for ( int i = 0; i < 5; i++ )
		m_pCBox_Condition->AddItem( GetConditionCodeString( i ), NULL );

	LoadControlSettings( "shadereditorui/vgui/sheet_condition.res" );
}
CSheet_Condition::~CSheet_Condition()
{
}

void CSheet_Condition::OnResetData()
{
	m_pCBox_Condition->ActivateItem( clamp( pData->GetInt( "i_combo_condition" ), 0, 4 ) );
}
void CSheet_Condition::OnApplyChanges()
{
	pData->SetInt( "i_combo_condition", m_pCBox_Condition->GetActiveItem() );
}
