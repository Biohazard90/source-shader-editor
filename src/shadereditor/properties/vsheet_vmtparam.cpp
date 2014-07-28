
#include "cbase.h"
#include "vSheets.h"


CSheet_VParam_Mutable::CSheet_VParam_Mutable(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_Params = new ComboBox( this, "cbox_params", 10, false );
	m_pCBox_Comps = new ComboBox( this, "cbox_types", 10, false );
	
	for ( int i = 0; i < AMT_VMT_MUTABLE; i++ )
		m_pCBox_Params->AddItem( VarArgs( "MUTABLE_%02i", i+1 ), NULL );
	for ( int i = 0; i < 4; i++ )
		m_pCBox_Comps->AddItem( ::GetVarTypeName( i ), NULL );

	m_pText_Values = new TextEntry(this, "constantvalues");

	LoadControlSettings( "shadereditorui/vgui/sheet_vparam_mutable.res" );
}
CSheet_VParam_Mutable::~CSheet_VParam_Mutable()
{
}

void CSheet_VParam_Mutable::OnResetData()
{
	m_pCBox_Params->ActivateItem( pData->GetInt( "i_vp_mutable_param" ) );
	m_pCBox_Comps->ActivateItem( pData->GetInt( "i_vp_mutable_nc" ) );
	m_pText_Values->SetText( pData->GetString( "szConstantString" ) );
}
void CSheet_VParam_Mutable::OnApplyChanges()
{
	pData->SetInt( "i_vp_mutable_param", m_pCBox_Params->GetActiveItem() );
	pData->SetInt( "i_vp_mutable_nc", m_pCBox_Comps->GetActiveItem() );

	char tmp[MAX_PATH];
	m_pText_Values->GetText( tmp, MAX_PATH );
	pData->SetString( "szConstantString", tmp );
}




CSheet_VParam_Static::CSheet_VParam_Static(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_Comps = new ComboBox( this, "cbox_types", 10, false );
	m_pTEntry_Name = new TextEntry( this, "tentry_name" );
	m_pText_Values = new TextEntry(this, "constantvalues");

	for ( int i = 0; i < 4; i++ )
		m_pCBox_Comps->AddItem( ::GetVarTypeName( i ), NULL );

	LoadControlSettings( "shadereditorui/vgui/sheet_vparam_static.res" );
}
CSheet_VParam_Static::~CSheet_VParam_Static()
{
}
void CSheet_VParam_Static::OnCommand( const char *cmd )
{
	BaseClass::OnCommand( cmd );
}
void CSheet_VParam_Static::OnResetData()
{
	m_pCBox_Comps->ActivateItem( pData->GetInt( "i_vp_static_nc" ) );
	m_pTEntry_Name->SetText( pData->GetString( "i_vp_static_name" ) );
	m_pText_Values->SetText( pData->GetString( "szConstantString" ) );
}
void CSheet_VParam_Static::OnApplyChanges()
{
	pData->SetInt( "i_vp_static_nc", m_pCBox_Comps->GetActiveItem() );

	char tmp[MAX_PATH];
	char tmp2[MAX_PATH];
	m_pTEntry_Name->GetText( tmp, MAX_PATH );
	::CleanupString( tmp, tmp2, MAX_PATH );
	pData->SetString( "i_vp_static_name", tmp2 );

	m_pText_Values->GetText( tmp, MAX_PATH );
	pData->SetString( "szConstantString", tmp );
}
