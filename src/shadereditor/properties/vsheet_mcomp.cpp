
#include "cbase.h"
#include "vSheets.h"


CSheet_MComp::CSheet_MComp(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_MType = new ComboBox( this, "cbox_mtype", 4, false );
	m_pCBox_MType->AddItem( "Matrix 3x3", NULL );
	m_pCBox_MType->AddItem( "Matrix 4x3", NULL );
	m_pCBox_MType->AddItem( "Matrix 4x4", NULL );

	m_pCheck_Columns = new CheckButton( this, "check_columns", "" );

	LoadControlSettings( "shadereditorui/vgui/sheet_mcompose.res" );
}
CSheet_MComp::~CSheet_MComp()
{
}

void CSheet_MComp::OnResetData()
{
	int mflag = pData->GetInt( "i_mcomp_matrix" );
	int item = 0;
	if ( mflag == HLSLVAR_MATRIX4X3 )
		item = 1;
	else if ( mflag == HLSLVAR_MATRIX4X4 )
		item = 2;
	m_pCBox_MType->ActivateItem( item );
	m_pCheck_Columns->SetSelected( !!pData->GetInt( "i_mcomp_c" ) );
}
void CSheet_MComp::OnApplyChanges()
{
	int item = m_pCBox_MType->GetActiveItem();
	int mflag = HLSLVAR_MATRIX3X3;
	if ( item == 1 )
		mflag = HLSLVAR_MATRIX4X3;
	else if ( item == 2 )
		mflag = HLSLVAR_MATRIX4X4;

	pData->SetInt( "i_mcomp_matrix", mflag );
	pData->SetInt( "i_mcomp_c", m_pCheck_Columns->IsSelected() ? 1 : 0 );
}
