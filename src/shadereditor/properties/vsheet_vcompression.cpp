
#include "cbase.h"
#include "vSheets.h"

static const char *pszCModes[] = {
	"Decompress normal",
	"Decompress normal and tangent",
};

CSheet_VCompression::CSheet_VCompression(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_CompressionModes = new ComboBox( this, "cbox_modes", 10, false );

	Assert( ARRAYSIZE( pszCModes ) == CNodeVCompression::COMPRESSION_MODE_COUNT );

	for ( int i = 0; i < CNodeVCompression::COMPRESSION_MODE_COUNT; i++ )
	{
		m_pCBox_CompressionModes->AddItem( pszCModes[i], NULL );
	}

	LoadControlSettings( "shadereditorui/vgui/sheet_vcompression.res" );
}

CSheet_VCompression::~CSheet_VCompression()
{
}

void CSheet_VCompression::OnResetData()
{
	m_pCBox_CompressionModes->ActivateItem( pData->GetInt( "iCompMode" ) );
}

void CSheet_VCompression::OnApplyChanges()
{
	pData->SetInt( "iCompMode", m_pCBox_CompressionModes->GetActiveItem() );
}
