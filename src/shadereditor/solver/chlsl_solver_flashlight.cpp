
#include "cbase.h"
#include "editorcommon.h"


void CHLSL_Solver_Flashlight::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *var_wpos = GetSourceVar( 0 );
	CHLSL_Var *var_wnormal = GetSourceVar( 1 );
	CHLSL_Var *var_fvppos = GetSourceVar( 2 );
	CHLSL_Var *var_screenpos = GetSourceVar( 3 );
	CHLSL_Var *var_eyedir = m_bSpecular ? GetSourceVar( 4 ) : NULL;
	CHLSL_Var *var_specexp = m_bSpecular ? GetSourceVar( 5 ) : NULL;
	CHLSL_Var *var_fresnel = m_bSpecular ? GetSourceVar( 6 ) : NULL;

	CHLSL_Var *var_diffuse = GetTargetVar( 0 );
	CHLSL_Var *var_speclight = m_bSpecular ? GetTargetVar( 1 ) : NULL;

	char tmp[MAXTARGC];
	if ( !m_bSpecular )
	{
		var_diffuse->DeclareMe(context);

		Q_snprintf( tmp, MAXTARGC, "%s = DoFlashlight( g_cFlashlightPos.xyz, %s, %s, %s,\n\t\t\
g_cFlashlightAttenuationFactors.xyz, g_cFlashlightAttenuationFactors.w,\n\t\t\
_gSampler_Flashlight_Cookie, _gSampler_Flashlight_Depth, _gSampler_Flashlight_Random,\n\t\t\
FLASHLIGHTDEPTHFILTERMODE, FLASHLIGHTSHADOWS, true,\n\t\t\
%s, false, g_cShadowTweaks );\n",
			var_diffuse->GetName(),
			var_wpos->GetName(), var_fvppos->GetName(), var_wnormal->GetName(),
			var_screenpos->GetName() );
	}
	else
	{
		var_diffuse->DeclareMe(context,true);
		var_speclight->DeclareMe(context,true);

		Q_snprintf( tmp, MAXTARGC, "DoSpecularFlashlight( g_cFlashlightPos.xyz, %s, %s, %s,\n\t\t\
g_cFlashlightAttenuationFactors.xyz, g_cFlashlightAttenuationFactors.w,\n\t\t\
_gSampler_Flashlight_Cookie, _gSampler_Flashlight_Depth, _gSampler_Flashlight_Random,\n\t\t\
FLASHLIGHTDEPTHFILTERMODE, FLASHLIGHTSHADOWS, true,\n\t\t\
%s, %s, %s, %s, g_cShadowTweaks,\n\t\t\
%s, %s );\n",
			var_wpos->GetName(), var_fvppos->GetName(), var_wnormal->GetName(),
			var_screenpos->GetName(), var_specexp->GetName(), var_eyedir->GetName(), var_fresnel->GetName(),
			var_diffuse->GetName(), var_speclight->GetName() );
	}
	/*
void DoSpecularFlashlight( float3 flashlightPos, float3 worldPos, float4 flashlightSpacePosition, float3 worldNormal,  
					float3 attenuationFactors, float farZ, sampler FlashlightSampler, sampler FlashlightDepthSampler, sampler RandomRotationSampler,
					int nShadowLevel, bool bDoShadows, bool bAllowHighQuality, const float2 vScreenPos, const float fSpecularExponent, const float3 vEyeDir,
					 float fFresnel, float4 vShadowTweaks,

					out float3 diffuseLighting, out float3 specularLighting )
	*/
	//Q_snprintf( tmp, MAXTARGC, "%s = %s;\n",
	//	tg->GetName(),src1->GetName() );

	context.buf_code.PutString( tmp );
}
void CHLSL_Solver_Flashlight::SetState( bool spec )
{
	m_bSpecular = spec;
}
void CHLSL_Solver_Flashlight::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_UTILITY_ASSIGN, 1, 1 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_UTILITY_ASSIGN ) );
	UpdateTargetVarToReflectMapIndex( 0 );
	UpdateTargetVarToReflectMapIndex( 1 );
}
void CHLSL_Solver_Flashlight::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_FLASHLIGHT_DATA;
	List.hList_EConstants.AddToTail( ec );

	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_FLASHLIGHT_FILTER_MODE ) );
	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_FLASHLIGHT_ENABLED ) );
	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_FLASHLIGHT_DO_SHADOWS ) );


	SimpleTexture *sampler = new SimpleTexture();
	sampler->iTextureMode = HLSLTEX_FLASHLIGHT_COOKIE;
	sampler->bSRGB = true;
	sampler->m_hTargetNodes.AddToTail( new HNODE( GetData().iNodeIndex ) );
	List.hList_Textures.AddToTail( sampler );

	sampler = new SimpleTexture();
	sampler->iTextureMode = HLSLTEX_FLASHLIGHT_DEPTH;
	sampler->m_hTargetNodes.AddToTail( new HNODE( GetData().iNodeIndex ) );
	List.hList_Textures.AddToTail( sampler );

	sampler = new SimpleTexture();
	sampler->iTextureMode = HLSLTEX_FLASHLIGHT_RANDOM;
	sampler->m_hTargetNodes.AddToTail( new HNODE( GetData().iNodeIndex ) );
	List.hList_Textures.AddToTail( sampler );
}



void CHLSL_Solver_Flashlight_Pos::Render( Preview2DContext &c )
{
	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( 0, 0, 0, 0 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}
void CHLSL_Solver_Flashlight_Pos::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_FLASHLIGHT_DATA;
	List.hList_EConstants.AddToTail( ec );

	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_FLASHLIGHT_ENABLED ) );
	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_FLASHLIGHT_DO_SHADOWS ) );
}
void CHLSL_Solver_Flashlight_Pos::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	target->SetName( "float3( g_cFlashlightPos.xyz )", true );
}