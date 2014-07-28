
#include "cbase.h"
#include "vSheets.h"


CSheet_Swizzle::CSheet_Swizzle(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	m_pText_Values = new TextEntry(this, "name");

	LoadControlSettings( "shadereditorui/vgui/sheet_swizzle.res" );
}
CSheet_Swizzle::~CSheet_Swizzle()
{
}

void CSheet_Swizzle::OnCommand( const char *cmd )
{
	BaseClass::OnCommand( cmd );
}

void CSheet_Swizzle::OnResetData()
{
	m_pText_Values->SetText( pData->GetString( "swizzleString" ) );
}
void CSheet_Swizzle::OnApplyChanges()
{
	char name[MAX_PATH];
	m_pText_Values->GetText( name, MAX_PATH );
	pData->SetString( "swizzleString", name );
}
