
#include "cbase.h"
#include "vSheets.h"


CSheet_Final::CSheet_Final(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_Tonemap = new ComboBox( this, "cbox_tonemap", 4, false );
	m_pCBox_Tonemap->AddItem( "None", NULL );
	m_pCBox_Tonemap->AddItem( "Linear", NULL );
	m_pCBox_Tonemap->AddItem( "Gamma", NULL );
	m_pCheck_WriteDepth = new CheckButton(this, "slight","");

	LoadControlSettings( "shadereditorui/vgui/sheet_final.res" );
}
CSheet_Final::~CSheet_Final()
{
}

void CSheet_Final::OnResetData()
{
	m_pCBox_Tonemap->ActivateItem( pData->GetInt( "i_final_tonemap" ) );
	m_pCheck_WriteDepth->SetSelected( ( pData->GetInt( "i_final_wdepth" ) != 0 ) );
}
void CSheet_Final::OnApplyChanges()
{
	pData->SetInt( "i_final_tonemap", m_pCBox_Tonemap->GetActiveItem() );
	pData->SetInt( "i_final_wdepth", m_pCheck_WriteDepth->IsSelected() ? 1 : 0 );
}