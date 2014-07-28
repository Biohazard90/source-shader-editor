
#include "cbase.h"
#include "vSheets.h"


CSheet_TexTransform::CSheet_TexTransform(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCheck_Center = new CheckButton( this, "check_center", "Custom center" );
	m_pCheck_Rotation = new CheckButton( this, "check_rot", "Rotation" );
	m_pCheck_Scale = new CheckButton( this, "check_scale", "Scale" );
	m_pCheck_Translation = new CheckButton( this, "check_translation", "Translation" );

	LoadControlSettings( "shadereditorui/vgui/sheet_textransform.res" );
}
CSheet_TexTransform::~CSheet_TexTransform()
{
}

void CSheet_TexTransform::OnResetData()
{
	m_pCheck_Center->SetSelected( ( pData->GetInt( "iTexTrans_center" ) != 0 ) );
	m_pCheck_Rotation->SetSelected( ( pData->GetInt( "iTexTrans_rot" ) != 0 ) );
	m_pCheck_Scale->SetSelected( ( pData->GetInt( "iTexTrans_scale" ) != 0 ) );
	m_pCheck_Translation->SetSelected( ( pData->GetInt( "iTexTrans_trans" ) != 0 ) );
}
void CSheet_TexTransform::OnApplyChanges()
{
	pData->SetInt( "iTexTrans_center", m_pCheck_Center->IsSelected() ? 1 : 0 );
	pData->SetInt( "iTexTrans_rot", m_pCheck_Rotation->IsSelected() ? 1 : 0 );
	pData->SetInt( "iTexTrans_scale", m_pCheck_Scale->IsSelected() ? 1 : 0 );
	pData->SetInt( "iTexTrans_trans", m_pCheck_Translation->IsSelected() ? 1 : 0 );
}
