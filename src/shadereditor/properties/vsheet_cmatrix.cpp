
#include "cbase.h"
#include "vSheets.h"


CSheet_CMatrix::CSheet_CMatrix(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_MType = new ComboBox( this, "cbox_mtype", 10, false );
	for ( int i = 0; i < CMATRIX_LAST; i++ )
		m_pCBox_MType->AddItem( GetCMatrixInfo( i )->szCanvasName, NULL );

	LoadControlSettings( "shadereditorui/vgui/sheet_cmatrix.res" );
}
CSheet_CMatrix::~CSheet_CMatrix()
{
}

void CSheet_CMatrix::OnResetData()
{
	m_pCBox_MType->ActivateItem( pData->GetInt( "i_c_matrix" ) );
}
void CSheet_CMatrix::OnApplyChanges()
{
	pData->SetInt( "i_c_matrix", m_pCBox_MType->GetActiveItem() );
}
