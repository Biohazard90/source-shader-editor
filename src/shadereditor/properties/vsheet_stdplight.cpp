
#include "cbase.h"
#include "vSheets.h"


CSheet_Std_PLight::CSheet_Std_PLight(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCheck_StaticLighting = new CheckButton(this, "slight","");
	m_pCheck_AOTerm = new CheckButton(this, "AO","");
	m_pCheck_Halflambert = new CheckButton(this, "halfl","");

	LoadControlSettings( "shadereditorui/vgui/sheet_std_plight.res" );
}
CSheet_Std_PLight::~CSheet_Std_PLight()
{
}

void CSheet_Std_PLight::OnResetData()
{
	m_pCheck_StaticLighting->SetSelected( ( pData->GetInt( "i_plight_static" ) != 0 ) );
	m_pCheck_AOTerm->SetSelected( ( pData->GetInt( "i_plight_ao" ) != 0 ) );
	m_pCheck_Halflambert->SetSelected( ( pData->GetInt( "i_plight_hl" ) != 0 ) );
}
void CSheet_Std_PLight::OnApplyChanges()
{
	pData->SetInt( "i_plight_static", m_pCheck_StaticLighting->IsSelected() ? 1 : 0 );
	pData->SetInt( "i_plight_ao", m_pCheck_AOTerm->IsSelected() ? 1 : 0 );
	pData->SetInt( "i_plight_hl", m_pCheck_Halflambert->IsSelected() ? 1 : 0 );
}