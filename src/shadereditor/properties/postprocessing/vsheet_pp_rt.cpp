
#include "cbase.h"
#include "vSheets.h"

static const char *pszRTNames[] = {
	"_rt_FullFrameFB",
	"_rt_PowerOfTwoFB",
	"_rt_SmallHDR0",
	"_rt_SmallHDR1",
	"_rt_SmallFB0",
	"_rt_SmallFB1",
	"_rt_WaterReflection",
	"_rt_WaterRefraction",
};
static const int iNumRTNames = ARRAYSIZE( pszRTNames );

CSheet_PP_RT::CSheet_PP_RT(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent )
	: BaseClass( pNode, view, data, parent )
{
	SetParent( parent );

	//m_pText_RTName = new TextEntry( this, "textentry_rtname" );
	m_pCBox_RTName = new ComboBox( this, "textentry_rtname", 20, true );

	for ( int i = 0; i < iNumRTNames; i++ )
	{
		ITexture *pTex = materials->FindTexture( pszRTNames[i], TEXTURE_GROUP_OTHER, false );
		if ( IsErrorTexture( pTex ) )
			continue;

		m_pCBox_RTName->AddItem( pszRTNames[i], NULL );
	}

	for ( int i = 0; i < GetRTManager()->GetNumRTs(); i++ )
		m_pCBox_RTName->AddItem( GetRTManager()->GetRT(i)->GetName(), NULL );

	m_pImgRef = new Panel( this, "img_ref" );
	m_pImgRef->SetPaintBackgroundEnabled( false );

	LoadControlSettings( "shadereditorui/vgui/sheet_pp_rt.res" );

	SetupVguiTex( m_iVguiPaint_Texture, "shadereditor/vguiTexSample" );
}

CSheet_PP_RT::~CSheet_PP_RT()
{
}

void CSheet_PP_RT::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pImgRef->SetBorder(pScheme->GetBorder("DepressedButtonBorder"));
}

void CSheet_PP_RT::OnResetData()
{
	m_pCBox_RTName->SetText( pData->GetString( "szRTName" ) );
}

void CSheet_PP_RT::OnApplyChanges()
{
	char tmp[MAX_PATH*4];
	m_pCBox_RTName->GetText( tmp, sizeof(tmp) );

	pData->SetString( "szRTName", tmp );
}

void CSheet_PP_RT::OnTextChanged( KeyValues *pKV )
{
	Panel *p = (Panel*)pKV->GetPtr("panel");

	if ( !p )
		return;

	if ( p == m_pCBox_RTName )
	{
		char tmp[MAX_PATH*4];
		m_pCBox_RTName->GetText( tmp, sizeof(tmp) );

		IMaterial *pMat = materials->FindMaterial( "shadereditor/vguiTexSample", TEXTURE_GROUP_OTHER );
		if ( !pMat )
			return;

		bool bFound = false;
		IMaterialVar *pBaseTex = pMat->FindVar( "$basetexture", &bFound );
		if ( !bFound )
			return;

		ITexture *tex = materials->FindTexture( tmp, TEXTURE_GROUP_OTHER, false );
		if ( IsErrorTexture(tex) )
			return;

		pBaseTex->SetTextureValue( tex );
#ifdef SHADER_EDITOR_DLL_2006
		pMat->Refresh();
#else
		pMat->RefreshPreservingMaterialVars();
#endif
		pMat->RecomputeStateSnapshots();
	}
}

void CSheet_PP_RT::Paint()
{
	BaseClass::Paint();

	int x,y,sx,sy;
	m_pImgRef->GetBounds( x,y,sx,sy);

	surface()->DrawSetColor( Color( 255,255,255,255) );
	surface()->DrawSetTexture( m_iVguiPaint_Texture );
	surface()->DrawTexturedRect( x,y,x+sx,y+sy);
}