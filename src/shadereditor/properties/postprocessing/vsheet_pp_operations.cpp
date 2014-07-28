
#include "cbase.h"
#include "editorCommon.h"

CSheet_PP_ClearBuff::CSheet_PP_ClearBuff(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCheck_ClearColor = new CheckButton( this, "check_do_col", "Clear color" );
	m_pCheck_ClearDepth = new CheckButton( this, "check_do_depth", "Clear depth" );
	m_pText_Values = new TextEntry(this, "constantvalues");

	LoadControlSettings( "shadereditorui/vgui/sheet_pp_clear_buffers.res" );
}

CSheet_PP_ClearBuff::~CSheet_PP_ClearBuff()
{
}

void CSheet_PP_ClearBuff::OnResetData()
{
	m_pText_Values->SetText( pData->GetString( "szConstantString" ) );
	m_pCheck_ClearColor->SetSelected( !!pData->GetInt( "iClear_Color" ) );
	m_pCheck_ClearDepth->SetSelected( !!pData->GetInt( "iClear_Depth" ) );
}

void CSheet_PP_ClearBuff::OnApplyChanges()
{
	char values_raw[MAX_PATH];
	m_pText_Values->GetText( values_raw, MAX_PATH );
	pData->SetString( "szConstantString", values_raw );

	pData->SetInt( "iClear_Color", m_pCheck_ClearColor->IsSelected() ? 1 : 0 );
	pData->SetInt( "iClear_Depth", m_pCheck_ClearDepth->IsSelected() ? 1 : 0 );
}