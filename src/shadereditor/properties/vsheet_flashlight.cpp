
#include "cbase.h"
#include "vSheets.h"


CSheet_Flashlight::CSheet_Flashlight(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCheck_Specular = new CheckButton(this, "check_spec","");

	LoadControlSettings( "shadereditorui/vgui/sheet_flashlight.res" );
}
CSheet_Flashlight::~CSheet_Flashlight()
{
}

void CSheet_Flashlight::OnResetData()
{
	m_pCheck_Specular->SetSelected( ( pData->GetInt( "i_flashlight_Spec" ) != 0 ) );
}
void CSheet_Flashlight::OnApplyChanges()
{
	pData->SetInt( "i_flashlight_Spec", m_pCheck_Specular->IsSelected() ? 1 : 0 );
}