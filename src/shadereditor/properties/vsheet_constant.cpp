
#include "cbase.h"
#include "vSheets.h"


CSheet_Constant::CSheet_Constant(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pText_Values = new TextEntry(this, "constantvalues");

	LoadControlSettings( "shadereditorui/vgui/sheet_constant.res" );
}
CSheet_Constant::~CSheet_Constant()
{
}

void CSheet_Constant::OnCommand( const char *cmd )
{
	BaseClass::OnCommand( cmd );
}

void CSheet_Constant::OnResetData()
{
	m_pText_Values->SetText( pData->GetString( "szConstantString" ) );
}
void CSheet_Constant::OnApplyChanges()
{
	char values_raw[MAX_PATH];
	m_pText_Values->GetText( values_raw, MAX_PATH );

	pData->SetString( "szConstantString", values_raw );
}



CSheet_Random::CSheet_Random(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_NumComp = new ComboBox(this, "vartype", 6, false);
	for (int i = 0; i < 4; i++ )
		m_pCBox_NumComp->AddItem( ::GetVarTypeName( i ), NULL );

	m_pTEntry_ValueMin = new TextEntry( this, "value_min" );
	m_pTEntry_ValueMax = new TextEntry( this, "value_max" );

	LoadControlSettings( "shadereditorui/vgui/sheet_random.res" );
}
CSheet_Random::~CSheet_Random()
{
}
void CSheet_Random::OnResetData()
{
	m_pCBox_NumComp->ActivateItem( pData->GetInt( "i_num_comps" ) );
	m_pTEntry_ValueMin->SetText( VarArgs( "%f", pData->GetFloat( "fl_value_min" ) ) );
	m_pTEntry_ValueMax->SetText( VarArgs( "%f", pData->GetFloat( "fl_value_max" ) ) );
}
void CSheet_Random::OnApplyChanges()
{
	pData->SetInt( "i_num_comps", m_pCBox_NumComp->GetActiveItem() );

	char tmp[MAX_PATH];
	m_pTEntry_ValueMin->GetText( tmp, MAX_PATH );
	float valtmp = atof( tmp );
	pData->SetFloat( "fl_value_min", valtmp );

	m_pTEntry_ValueMax->GetText( tmp, MAX_PATH );
	valtmp = atof( tmp );
	pData->SetFloat( "fl_value_max", valtmp );
}



CSheet_EnvCTexelsize::CSheet_EnvCTexelsize(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_FBScaling = new ComboBox(this, "fbscaling", 6, false);
	m_pCBox_FBScaling->AddItem( "Full", NULL );
	m_pCBox_FBScaling->AddItem( "Half", NULL );
	m_pCBox_FBScaling->AddItem( "Quarter", NULL );

	LoadControlSettings( "shadereditorui/vgui/sheet_fbtexelsize.res" );
}
CSheet_EnvCTexelsize::~CSheet_EnvCTexelsize()
{
}
void CSheet_EnvCTexelsize::OnResetData()
{
	int index = (int)( pData->GetFloat( "flSmartVal0" ) / 2.0f );
	Assert( index >= 0 && index <= 2 );

	m_pCBox_FBScaling->ActivateItem( index );
}
void CSheet_EnvCTexelsize::OnApplyChanges()
{
	pData->SetFloat( "flSmartVal0", m_pCBox_FBScaling->GetActiveItem() * 2.0f );
}