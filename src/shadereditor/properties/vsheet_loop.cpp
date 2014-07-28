
#include "cbase.h"
#include "vSheets.h"


CSheet_Loop::CSheet_Loop(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_Condition = new ComboBox( this, "cbox_condition", 4, false );
	for ( int i = 0; i < 4; i++ )
		m_pCBox_Condition->AddItem( GetConditionCodeString( i ), NULL );

	m_pText_Start = new TextEntry( this, "text_start" );
	m_pText_End = new TextEntry( this, "text_end" );

	LoadControlSettings( "shadereditorui/vgui/sheet_loop.res" );
}
CSheet_Loop::~CSheet_Loop()
{
}

void CSheet_Loop::OnResetData()
{
	m_pCBox_Condition->ActivateItem( clamp( pData->GetInt( "loop_condition" ), 0, 3 ) );
	m_pText_Start->SetText( VarArgs( "%i", pData->GetInt( "loop_value_min" ) ) );
	m_pText_End->SetText( VarArgs( "%i", pData->GetInt( "loop_value_max" ) ) );
}
void CSheet_Loop::OnApplyChanges()
{
	int _min, _max;
	char tmp[MAX_PATH];
	m_pText_Start->GetText( tmp, MAX_PATH );
	_min = atoi( tmp );
	m_pText_End->GetText( tmp, MAX_PATH );
	_max = atoi( tmp );

	pData->SetInt( "loop_condition", m_pCBox_Condition->GetActiveItem() );
	pData->SetInt( "loop_value_min", _min );
	pData->SetInt( "loop_value_max", _max );
}
