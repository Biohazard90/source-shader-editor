
#include "cbase.h"
#include "editorcommon.h"


void CHLSL_Solver_Parallax::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	const char *pszFuncPOM = "CalcParallaxUV_POM";
	const char *pszFuncRelief = "CalcParallaxUV_Relief";
	const char *pszFalse = "false";
	const char *pszTrue = "true";

	int iCurJack = 0;
	CHLSL_Var *var_uv = GetSourceVar( iCurJack );iCurJack++;
	CHLSL_Var *var_viewts = GetSourceVar( iCurJack );iCurJack++;
	CHLSL_Var *var_prlxamt = GetSourceVar( iCurJack );iCurJack++;
	CHLSL_Var *var_wpos = NULL;
	//if ( m_PSetup.bGetOffset )
	{
		var_wpos = GetSourceVar( iCurJack );
		iCurJack++;
	}
	CHLSL_Var *var_wnormal = GetSourceVar( iCurJack );iCurJack++;
	CHLSL_Var *var_eyedir = GetSourceVar( iCurJack );iCurJack++;
	CHLSL_Var *var_sampler_heightmap = GetSourceVar( iCurJack );iCurJack++;

	CHLSL_Var *var_uv_out = GetTargetVar( 0 );
	CHLSL_Var *var_st_out = GetTargetVar( 1 ); //m_PSetup.bGetOffset ? GetTargetVar( 1 ) : NULL;

	char tmp[MAXTARGC];
	//if ( m_PSetup.bGetOffset )
	var_st_out->DeclareMe(context, true);
	var_uv_out->DeclareMe(context);

		Q_snprintf( tmp, MAXTARGC, "%s = %s( %s, %s, %s, %s, %s,\n\t\t\
%s, %i, %i, %i, %s,\n\t\t\
%s, %s, %s );\n",
			var_uv_out->GetName(),
			( ( m_PSetup.iParallaxMode == ParallaxConfig_t::PCFG_POM ) ? pszFuncPOM : pszFuncRelief ),
			var_uv->GetName(),
			var_viewts->GetName(),
			var_prlxamt->GetName(),
			var_wnormal->GetName(),
			var_eyedir->GetName(),
			var_sampler_heightmap->GetName(),
			m_PSetup.iSamplesMin,
			m_PSetup.iSamplesMax,
			m_PSetup.iBinaryMax,
			( var_wpos ? var_wpos->GetName() : "float3(0,0,0)" ),
			var_st_out->GetName(),
			( m_PSetup.bGradientLookup ? pszTrue : pszFalse ),
			( m_PSetup.bGetOffset ? pszTrue : pszFalse )
			);


	/*
( float2 inTexCoord, float3 vViewTS, float flParallaxAmt, float3 vNormal, 
													float3 vViewW, sampler HeightMapSampler,
													const int samples_min, const int samples_max, const int binary_max,
													float3 WPos, out float3 worldSpaceOffset,
													const bool bDoGradient, const bool bGetOffset )
	*/
//	if ( m_bDoDTest )
//		Q_snprintf( tmp, MAXTARGC, "%s = CalcParallaxedTexCoord_Editor_DepthTest( %s, %s, %s, %s, %s,\n\t\t\
//%s, %i, %i, %s, %s );\n",
//			var_uv_out->GetName(),
//			var_uv->GetName(),
//			var_viewts->GetName(),
//			var_prlxamt->GetName(),
//			var_wnormal->GetName(),
//			var_eyedir->GetName(),
//			var_sampler_heightmap->GetName(),
//			//m_iSamplesMin, m_iSamplesMax,
//			var_wpos->GetName(),
//			var_st_out->GetName()
//			);
//	else
//		Q_snprintf( tmp, MAXTARGC, "%s = CalcParallaxedTexCoord_Editor_Fast( %s, %s, %s, %s, %s,\n\t\t\
//%s, %i, %i );\n",
//			var_uv_out->GetName(),
//			var_uv->GetName(),
//			var_viewts->GetName(),
//			var_prlxamt->GetName(),
//			var_wnormal->GetName(),
//			var_eyedir->GetName(),
//			var_sampler_heightmap->GetName(),
//			//m_iSamplesMin, m_iSamplesMax );
//			);

	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_Parallax::SetState( ParallaxConfig_t data )
{
	m_PSetup = data;
}

void CHLSL_Solver_Parallax::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_UTILITY_DECLARE, 0, 0 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_UTILITY_DECLARE ) );
	UpdateTargetVarToReflectMapIndex( 0 );
	UpdateTargetVarToReflectMapIndex( 1 );
}





void CHLSL_Solver_Parallax_StdShadow::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *var_uv = GetSourceVar( 0 );
	CHLSL_Var *var_uv_prlx = GetSourceVar( 1 );
	CHLSL_Var *var_lightts = GetSourceVar( 2 );
	CHLSL_Var *var_prlxamt = GetSourceVar( 3 );
	CHLSL_Var *var_wpos = GetSourceVar( 4 );
	CHLSL_Var *var_softening = GetSourceVar( 5 );
	CHLSL_Var *var_sampler_heightmap = GetSourceVar( 6 );

	CHLSL_Var *var_lout = GetTargetVar( 0 );

	char tmp[MAXTARGC];
	var_lout->DeclareMe(context);
	//Q_snprintf( tmp, MAXTARGC, "%s = CalcParallaxedShadows( %s, %s, %s, %s );\n",
	Q_snprintf( tmp, MAXTARGC, "%s = CalcParallaxedShadows_OneLight( %s, %s, %s,\n\t\t%s, %s, %s, %s );\n",
		var_lout->GetName(),
		var_uv->GetName(),
		var_uv_prlx->GetName(),
		var_lightts->GetName(),
		var_wpos->GetName(),
		var_prlxamt->GetName(),
		var_softening->GetName(),
		var_sampler_heightmap->GetName() );

	context.buf_code.PutString( tmp );
}
void CHLSL_Solver_Parallax_StdShadow::Render( Preview2DContext &c )
{
	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( 1, 1, 1, 1 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}