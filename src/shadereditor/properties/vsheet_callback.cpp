
#include "cbase.h"
#include "vSheets.h"


CSheet_Callback::CSheet_Callback(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_Callbacks = new ComboBox( this, "cbox_callbacks", 10, false );

	for ( int i = 0; i < shaderEdit->GetNumCallbacks(); i++ )
	{
		_clCallback *c = shaderEdit->GetCallback( i );
		m_pCBox_Callbacks->AddItem( c->name, NULL );
	}

	LoadControlSettings( "shadereditorui/vgui/sheet_callback.res" );
}
CSheet_Callback::~CSheet_Callback()
{
}

void CSheet_Callback::OnResetData()
{
	const char *callbackname = pData->GetString( "sz_callbackname" );
	int target = -1;
	for ( int i = 0; i < m_pCBox_Callbacks->GetItemCount(); i++ )
	{
		char tmp[ MAX_PATH ];
		m_pCBox_Callbacks->GetItemText( i, tmp, MAX_PATH );
		if ( !Q_stricmp( tmp, callbackname ) )
			target = i;
	}

	if ( target >= 0 )
		m_pCBox_Callbacks->ActivateItem( target );
}
void CSheet_Callback::OnApplyChanges()
{
	if ( !m_pCBox_Callbacks->GetItemCount() )
		return;

	int ID = m_pCBox_Callbacks->GetActiveItem();
	if ( ID < 0 || ID >= shaderEdit->GetNumCallbacks() )
		return;

	char tmp[ MAX_PATH ];
	m_pCBox_Callbacks->GetItemText( ID, tmp, MAX_PATH );
	pData->SetString( "sz_callbackname", tmp );

	_clCallback *c = shaderEdit->GetCallback( ID );
	pData->SetInt( "i_numc", c->numComps );
}
