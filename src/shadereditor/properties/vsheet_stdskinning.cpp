
#include "cbase.h"
#include "vSheets.h"


CSheet_Std_Skinning::CSheet_Std_Skinning(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBoxMode = new ComboBox( this, "cbox_mode", 6, false );
	m_pCBoxMode->AddItem( "Position", NULL );
	m_pCBoxMode->AddItem( "Position and Normal", NULL );
	m_pCBoxMode->AddItem( "Pos, Normal, Tangentspace", NULL );

	LoadControlSettings( "shadereditorui/vgui/sheet_std_skinning.res" );
}
CSheet_Std_Skinning::~CSheet_Std_Skinning()
{
}

void CSheet_Std_Skinning::OnResetData()
{
	m_pCBoxMode->ActivateItem( pData->GetInt( "i_vskinning_mode" ) );
}
void CSheet_Std_Skinning::OnApplyChanges()
{
	pData->SetInt( "i_vskinning_mode", m_pCBoxMode->GetActiveItem() );
}