
#include "cbase.h"
#include "vSheets.h"

static const char *pszSizingModes[] = {
	"Full frame buffer",
	"Half frame buffer",
	"Quarter frame buffer",
};
static const int iSizingModes = ARRAYSIZE( pszSizingModes );

CSheet_PP_DrawMat::CSheet_PP_DrawMat(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	m_pCBox_SizingTg = new ComboBox( this, "cbox_sizingrt", 20, true );
	m_pCBox_SizingSrc = new ComboBox( this, "cbox_sizingsrc", 20, true );
	m_pCheck_PushTarget = new CheckButton( this, "checkpushtg", "Push target for rendering" );

	m_pTE_Destx = new TextEntry( this, "compensate_dest_x" );
	m_pTE_Desty = new TextEntry( this, "compensate_dest_y" );
	m_pTE_Destw = new TextEntry( this, "compensate_dest_w" );
	m_pTE_Desth = new TextEntry( this, "compensate_dest_h" );

	m_pTE_Srcx0 = new TextEntry( this, "compensate_src_x0" );
	m_pTE_Srcy0 = new TextEntry( this, "compensate_src_y0" );
	m_pTE_Srcx1 = new TextEntry( this, "compensate_sourceuv_x" );
	m_pTE_Srcy1 = new TextEntry( this, "compensate_sourceuv_y" );

	m_pTE_Srcw = new TextEntry( this, "compensate_src_w" );
	m_pTE_Srch = new TextEntry( this, "compensate_src_h" );

	for ( int i = 0; i < iSizingModes; i++ )
	{
		m_pCBox_SizingTg->AddItem( pszSizingModes[i], NULL );
		m_pCBox_SizingSrc->AddItem( pszSizingModes[i], NULL );
	}

	LoadControlSettings( "shadereditorui/vgui/sheet_pp_drawmat.res" );
}

CSheet_PP_DrawMat::~CSheet_PP_DrawMat()
{
}

void CSheet_PP_DrawMat::OnResetData()
{
	int iTg = pData->GetInt( "iSizingMode_Tg" );
	int iSrc = pData->GetInt( "iSizingMode_Src" );

	Assert( iTg >= 0 && iTg < iSizingModes );
	Assert( iSrc >= 0 && iSrc < iSizingModes );

	m_pCBox_SizingTg->ActivateItem( iTg );
	m_pCBox_SizingSrc->ActivateItem( iSrc );

	m_pCBox_SizingTg->SetEnabled( !pData->GetInt( "iSHEETHINT_enable_tg" ) );
	m_pCBox_SizingSrc->SetEnabled( !pData->GetInt( "iSHEETHINT_enable_src" ) );

	m_pCheck_PushTarget->SetSelected( !!pData->GetInt( "iPushTg" ) );

	m_pTE_Destx->SetText( VarArgs( "%i", pData->GetInt( "iDst_x" ) ) );
	m_pTE_Desty->SetText( VarArgs( "%i", pData->GetInt( "iDst_y" ) ) );
	m_pTE_Destw->SetText( VarArgs( "%i", pData->GetInt( "iDst_w" ) ) );
	m_pTE_Desth->SetText( VarArgs( "%i", pData->GetInt( "iDst_h" ) ) );

	m_pTE_Srcx0->SetText( VarArgs( "%f", pData->GetFloat( "flSrcUV_x0" ) ) );
	m_pTE_Srcy0->SetText( VarArgs( "%f", pData->GetFloat( "flSrcUV_y0" ) ) );
	m_pTE_Srcx1->SetText( VarArgs( "%f", pData->GetFloat( "flSrcUV_x" ) ) );
	m_pTE_Srcy1->SetText( VarArgs( "%f", pData->GetFloat( "flSrcUV_y" ) ) );

	m_pTE_Srcw->SetText( VarArgs( "%i", pData->GetInt( "iSrc_w" ) ) );
	m_pTE_Srch->SetText( VarArgs( "%i", pData->GetInt( "iSrc_h" ) ) );
}

void CSheet_PP_DrawMat::OnApplyChanges()
{
	pData->SetInt( "iSizingMode_Tg", m_pCBox_SizingTg->GetActiveItem() );
	pData->SetInt( "iSizingMode_Src", m_pCBox_SizingSrc->GetActiveItem() );
	pData->SetInt( "iPushTg", m_pCheck_PushTarget->IsSelected() ? 1 : 0 );

	char tmp[MAX_PATH];
	m_pTE_Destx->GetText( tmp, sizeof(tmp) );
	pData->SetFloat( "iDst_x", atoi( tmp ) );
	m_pTE_Desty->GetText( tmp, sizeof(tmp) );
	pData->SetFloat( "iDst_y", atoi( tmp ) );
	m_pTE_Destw->GetText( tmp, sizeof(tmp) );
	pData->SetFloat( "iDst_w", atoi( tmp ) );
	m_pTE_Desth->GetText( tmp, sizeof(tmp) );
	pData->SetFloat( "iDst_h", atoi( tmp ) );

	m_pTE_Srcx0->GetText( tmp, sizeof(tmp) );
	pData->SetFloat( "flSrcUV_x0", atof( tmp ) );
	m_pTE_Srcy0->GetText( tmp, sizeof(tmp) );
	pData->SetFloat( "flSrcUV_y0", atof( tmp ) );
	m_pTE_Srcx1->GetText( tmp, sizeof(tmp) );
	pData->SetFloat( "flSrcUV_x", atof( tmp ) );
	m_pTE_Srcy1->GetText( tmp, sizeof(tmp) );
	pData->SetFloat( "flSrcUV_y", atof( tmp ) );

	m_pTE_Srcw->GetText( tmp, sizeof(tmp) );
	pData->SetFloat( "iSrc_w", atoi( tmp ) );
	m_pTE_Srch->GetText( tmp, sizeof(tmp) );
	pData->SetFloat( "iSrc_h", atoi( tmp ) );
}
