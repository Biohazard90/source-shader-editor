
#include "cbase.h"
#include "vSheets.h"


CSheet_Comment::CSheet_Comment(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pText_Values = new TextEntry(this, "constantvalues");

	LoadControlSettings( "shadereditorui/vgui/sheet_comment.res" );
}
CSheet_Comment::~CSheet_Comment()
{
}

void CSheet_Comment::OnCommand( const char *cmd )
{
	BaseClass::OnCommand( cmd );
}

void CSheet_Comment::OnResetData()
{
	m_pText_Values->SetText( pData->GetString( "szConstantString" ) );
}
void CSheet_Comment::OnApplyChanges()
{
	char values_raw[MAX_PATH];
	m_pText_Values->GetText( values_raw, MAX_PATH );

	pData->SetString( "szConstantString", values_raw );
}