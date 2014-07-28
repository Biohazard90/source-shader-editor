
#include "cbase.h"
#include "vSheets.h"


CSheet_Multiply::CSheet_Multiply(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCheck_MatrixAsRotation = new CheckButton( this, "rotationmatrix", "Matrix rotation only" );

	LoadControlSettings( "shadereditorui/vgui/sheet_multiply.res" );
}
CSheet_Multiply::~CSheet_Multiply()
{
}

void CSheet_Multiply::OnResetData()
{
	m_pCheck_MatrixAsRotation->SetSelected( pData->GetInt( "i_mat_rotation" ) != 0 );
}
void CSheet_Multiply::OnApplyChanges()
{
	pData->SetInt( "i_mat_rotation", m_pCheck_MatrixAsRotation->IsSelected() ? 1 : 0 );
}
