
#include "cbase.h"
#include "vSheets.h"


CSheet_Lightscale::CSheet_Lightscale(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_Lightscale = new ComboBox( this, "cbox_lightscale", LSCALE_MAX, false );
	for ( int i = 0; i < LSCALE_MAX; i++ )
		m_pCBox_Lightscale->AddItem( GetLightscaleGameString(i), NULL );

	LoadControlSettings( "shadereditorui/vgui/sheet_lightscale.res" );
}
CSheet_Lightscale::~CSheet_Lightscale()
{
}

void CSheet_Lightscale::OnResetData()
{
	m_pCBox_Lightscale->ActivateItem( clamp( pData->GetInt( "i_lscale_type" ), 0, (LSCALE_MAX-1) ) );
}
void CSheet_Lightscale::OnApplyChanges()
{
	pData->SetInt( "i_lscale_type", m_pCBox_Lightscale->GetActiveItem() );
}
