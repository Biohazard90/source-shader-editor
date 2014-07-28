
#include "cbase.h"
#include "editorCommon.h"

static const char *blendNames[] = {
	"None",
	"Blend enabled",
	"Blend inverted",
	"Blend additive",
	"Alpha test",
	"Alpha to coverage",
};
static const int blendIDx[] = {
	ABLEND_NONE,
	ABLEND_SIMPLE,
	ABLEND_SIMPLE_INVERTED,
	ABLEND_ADDITIVE,
	ABLEND_ALPHATEST,
	ABLEND_ALPHA2COVERAGE,
};

const int numAblendModes = ARRAYSIZE( blendNames );

CDialog_GeneralConfig::CDialog_GeneralConfig( CNodeView *n, Panel *parent ) : Frame( parent, "_config" )
{
	pNodeView = n;
	GenericShaderData *setup = &n->GetDataForModify();

	SetSizeable( false );
	SetVisible( true );

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	SetAutoDelete( true );
	Activate();

	m_pRadBut_Sm2 = new RadioButton( this, "rad_1", "SM 2.0b" );
	m_pRadBut_Sm3 = new RadioButton( this, "rad_2", "SM 3.0" );
	m_pRadBut_Sm2->SetSubTabPosition( 1 );
	m_pRadBut_Sm3->SetSubTabPosition( 1 );

	m_pCBut_DepthTest = new CheckButton( this, "depthtest", "Depth testing" );
	m_pCBut_DepthWrite = new CheckButton( this, "depthwrite", "Depth writing" );
	m_pCBut_sRGBWrite = new CheckButton( this, "srgbwrite", "write sRGB" );

	m_pCBox_AlphaBlend = new ComboBox( this, "alphablend", numAblendModes, false );
	
	for ( int i = 0; i < numAblendModes; i++ )
		m_pCBox_AlphaBlend->AddItem( blendNames[i], NULL );

	Assert( numAblendModes == m_pCBox_AlphaBlend->GetItemCount() );

	m_pCBox_Cullmode = new ComboBox( this, "cullmode", 5, false );
	m_pCBox_Cullmode->AddItem( "CW", NULL );
	m_pCBox_Cullmode->AddItem( "CCW", NULL );
	//m_pCBox_Cullmode->AddItem( "Double", NULL );
	m_pCBox_Cullmode->AddItem( "Off", NULL );

	LoadControlSettings("shadereditorui/vgui/dialog_config.res");

	if ( setup->shader->iShaderModel == SM_30 )
		m_pRadBut_Sm3->SetSelected( true );
	else
		m_pRadBut_Sm2->SetSelected( true );

	m_pCBut_DepthTest->SetSelected( setup->shader->iDepthtestmode == DEPTHTEST_NORMAL );
	m_pCBut_DepthWrite->SetSelected( setup->shader->iDepthwritemode == DEPTHWRITE_NORMAL );
	m_pCBox_Cullmode->ActivateItem( setup->shader->iCullmode );
	m_pCBut_sRGBWrite->SetSelected( setup->shader->bsRGBWrite );

	for ( int i = 0; i < numAblendModes; i++ )
	{
		if ( blendIDx[i] == setup->shader->iAlphablendmode )
		{
			m_pCBox_AlphaBlend->ActivateItem( i );
			break;
		}
	}

	SetTitle( "Shader settings", true );


	DoModal();

	SetDeleteSelfOnClose( true );
}

CDialog_GeneralConfig::~CDialog_GeneralConfig()
{
}

void CDialog_GeneralConfig::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( cmd, "save" ) )
	{
		OnSave();
		Close();
	}
	else if ( !Q_stricmp( cmd, "close" ) )
	{
		Close();
	}
}

void CDialog_GeneralConfig::PerformLayout()
{
	BaseClass::PerformLayout();
	SetCloseButtonVisible( false );

	SetSize( 400, 300 );
	MoveToCenterOfScreen();
}

void CDialog_GeneralConfig::OnSave()
{
	Assert( numAblendModes == m_pCBox_AlphaBlend->GetItemCount() && m_pCBox_AlphaBlend->GetActiveItem() < numAblendModes );

	GenericShaderData *setup = &pNodeView->GetDataForModify();
	setup->shader->iAlphablendmode = blendIDx[m_pCBox_AlphaBlend->GetActiveItem()];
	setup->shader->iCullmode = m_pCBox_Cullmode->GetActiveItem();
	setup->shader->iDepthtestmode = m_pCBut_DepthTest->IsSelected() ? DEPTHTEST_NORMAL : DEPTHTEST_OFF;
	setup->shader->iDepthwritemode = m_pCBut_DepthWrite->IsSelected() ? DEPTHWRITE_NORMAL : DEPTHWRITE_OFF;
	setup->shader->iShaderModel = m_pRadBut_Sm3->IsSelected() ? SM_30 : SM_20B;
	setup->shader->bsRGBWrite = m_pCBut_sRGBWrite->IsSelected();
	pNodeView->MakeSolversDirty();
}
