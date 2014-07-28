
#include "cbase.h"
#include "vSheets.h"


CSheet_Parallax::CSheet_Parallax(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	pTEntry_MinSamples = new TextEntry( this, "samp_min" );
	pTEntry_MaxSamples = new TextEntry( this, "samp_max" );
	pTEntry_BinarySamples = new TextEntry( this, "samp_binary" );

	pCheck_DepthTest = new CheckButton( this, "check_deptht", "Calc Geometry offset" );
	pCheck_GradientRead = new CheckButton( this, "check_grad", "Use gradient lookup" );

	pCBox_PrlxMode = new ComboBox( this, "cbox_mode", 6, false );
	pCBox_PrlxMode->AddItem( "Parallax occlusion mapping", NULL );
	pCBox_PrlxMode->AddItem( "Relief mapping", NULL );

	LoadControlSettings( "shadereditorui/vgui/sheet_parallax.res" );
}
CSheet_Parallax::~CSheet_Parallax()
{
}

void CSheet_Parallax::OnResetData()
{
	pTEntry_MinSamples->SetText( VarArgs( "%i", pData->GetInt( "i_samples_min" ) ) );
	pTEntry_MaxSamples->SetText( VarArgs( "%i", pData->GetInt( "i_samples_max" ) ) );
	pTEntry_BinarySamples->SetText( VarArgs( "%i", pData->GetInt( "i_samples_binary" ) ) );
	pCheck_DepthTest->SetSelected( !!pData->GetInt( "i_dtest" ) );
	pCheck_GradientRead->SetSelected( !!pData->GetInt( "i_gradient" ) );
	pCBox_PrlxMode->ActivateItem( pData->GetInt( "i_prlx_mode" ) );
}
void CSheet_Parallax::OnApplyChanges()
{
	char tmp[MAX_PATH];
	pTEntry_MinSamples->GetText( tmp, sizeof( tmp ) );
	int samp_min = atoi( tmp );
	pTEntry_MaxSamples->GetText( tmp, sizeof( tmp ) );
	int samp_max = atoi( tmp );
	pTEntry_BinarySamples->GetText( tmp, sizeof( tmp ) );
	int samp_bin = atoi( tmp );

	pData->SetInt( "i_samples_min", max( 1, samp_min ) );
	pData->SetInt( "i_samples_max", max( 1, samp_max ) );
	pData->SetInt( "i_samples_binary", max( 1, samp_bin ) );
	pData->SetInt( "i_dtest", pCheck_DepthTest->IsSelected() ? 1 : 0 );
	pData->SetInt( "i_gradient", pCheck_GradientRead->IsSelected() ? 1 : 0 );
	pData->SetInt( "i_prlx_mode", pCBox_PrlxMode->GetActiveItem() );
}

void CSheet_Parallax::OnTextChanged( KeyValues *pKV )
{
	UpdateLayout( (Panel*)pKV->GetPtr( "panel" ) );
}
void CSheet_Parallax::OnCheckButtonChecked( KeyValues *pKV )
{
	UpdateLayout( (Panel*)pKV->GetPtr( "panel" ) );
}
void CSheet_Parallax::UpdateLayout( Panel *pCaller )
{
	const bool bDoRelief = pCBox_PrlxMode->GetActiveItem() == ParallaxConfig_t::PCFG_RELIEF;

	const bool bBinary = bDoRelief;
	const bool bGradient = bDoRelief;
	const bool bLinear_min = !bDoRelief || pCheck_GradientRead->IsSelected();

	pTEntry_BinarySamples->SetEnabled( bBinary );
	pCheck_GradientRead->SetEnabled( bGradient );
	pTEntry_MinSamples->SetEnabled( bLinear_min );
}
