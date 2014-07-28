
#include "cbase.h"
#include "vSheets.h"


CSheet_Std_VLight::CSheet_Std_VLight(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCheck_StaticLighting = new CheckButton(this, "slight","");
	m_pCheck_Halflambert = new CheckButton(this, "halfl","");

	LoadControlSettings( "shadereditorui/vgui/sheet_std_vlight.res" );
}
CSheet_Std_VLight::~CSheet_Std_VLight()
{
}

void CSheet_Std_VLight::OnResetData()
{
	m_pCheck_StaticLighting->SetSelected( ( pData->GetInt( "i_vlight_static" ) != 0 ) );
	m_pCheck_Halflambert->SetSelected( ( pData->GetInt( "i_vlight_hl" ) != 0 ) );
}
void CSheet_Std_VLight::OnApplyChanges()
{
	pData->SetInt( "i_vlight_static", m_pCheck_StaticLighting->IsSelected() ? 1 : 0 );
	pData->SetInt( "i_vlight_hl", m_pCheck_Halflambert->IsSelected() ? 1 : 0 );
}

void CSheet_Std_VLight::OnCheckButtonChecked( KeyValues *pData )
{
	Panel *pCaller = ((Panel*)pData->GetPtr( "panel" ));
	bool bState = ( pData->GetInt( "state" ) != 0 );

	if ( pCaller && pCaller == m_pCheck_StaticLighting )
	{
		m_pCheck_Halflambert->SetEnabled( bState );
		if ( !bState )
			m_pCheck_Halflambert->SetSelected( bState );
	}
}