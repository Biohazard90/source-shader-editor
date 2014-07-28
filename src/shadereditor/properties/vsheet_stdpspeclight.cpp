
#include "cbase.h"
#include "vSheets.h"


CSheet_Std_PSpecLight::CSheet_Std_PSpecLight(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCheck_AOTerm = new CheckButton(this, "AO","");

	LoadControlSettings( "shadereditorui/vgui/sheet_std_pspeclight.res" );
}
CSheet_Std_PSpecLight::~CSheet_Std_PSpecLight()
{
}

void CSheet_Std_PSpecLight::OnResetData()
{
	m_pCheck_AOTerm->SetSelected( ( pData->GetInt( "i_plight_ao" ) != 0 ) );
}
void CSheet_Std_PSpecLight::OnApplyChanges()
{
	pData->SetInt( "i_plight_ao", m_pCheck_AOTerm->IsSelected() ? 1 : 0 );
}