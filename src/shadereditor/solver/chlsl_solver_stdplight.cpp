
#include "cbase.h"
#include "editorCommon.h"

void CHLSL_Solver_StdPLight::SetState( bool supportsStaticL, bool doHL, bool doAO )
{
	m_bStaticLighting = supportsStaticL;
	m_bAOTerm = doAO;
	m_bHalfLambert = doHL;
}

void CHLSL_Solver_StdPLight::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	char tmp[MAXTARGC];
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);

	CHLSL_Var *wpos = GetSourceVar( 0 );
	CHLSL_Var *wnormal = GetSourceVar( 1 );
	CHLSL_Var *vattn = GetSourceVar( 2 );
	CHLSL_Var *staticlighting = NULL;
	CHLSL_Var *aoterm = NULL;

	int curJack = 3;
	if ( m_bStaticLighting )
	{
		staticlighting = GetSourceVar( curJack );
		curJack++;
	}
	if ( m_bAOTerm )
	{
		aoterm = GetSourceVar( curJack );
		curJack++;
	}

	Q_snprintf( tmp, MAXTARGC, "%s = PixelShaderDoLighting( %s, %s, %s,\n\t\t%s, true, %s,\n\t\tg_cAmbientCube, NUM_LIGHTS, g_cLightInfo,\n\t\t%s, %s, %s );\n",
		tg->GetName(),
		wpos->GetName(),
		wnormal->GetName(),
		(staticlighting ? staticlighting->GetName() : "float3(0,0,0)"),
		(staticlighting ? "true" : "false"),
		vattn->GetName(),
		(m_bHalfLambert ? "true" : "false"),
		(aoterm ? "true" : "false"),
		(aoterm ? aoterm->GetName() : "1.0f") );

	context.buf_code.PutString( tmp );
	/*
float3 PixelShaderDoLighting( const float3 worldPos, const float3 worldNormal,
				   const float3 staticLightingColor, const bool bStaticLight,
				   const bool bAmbientLight, const float4 lightAtten, const float3 cAmbientCube[6],
				    const int nNumLights, PixelShaderLightInfo cLightInfo[3],
				   const bool bHalfLambert,
				   
				   // New optional/experimental parameters
				   const bool bDoAmbientOcclusion, const float fAmbientOcclusion )
	*/
}

void CHLSL_Solver_StdPLight::Render( Preview2DContext &c )
{
	IMaterialVar *ao_control = pEditorRoot->GetUVTargetParam( NPSOP_CALC_PSLIGHTING, 2 );
	ao_control->SetVecValue( m_bAOTerm ? 0 : 1 , 0, 0, 0 );

	SetUVParamBySourceVar( NPSOP_CALC_PSLIGHTING, 0, 1 ); // normal
	if ( m_bAOTerm )
		SetUVParamBySourceVar( NPSOP_CALC_PSLIGHTING, 1, m_bStaticLighting ? 4 : 3 ); // ao

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_PSLIGHTING ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_Solver_StdPLight::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_STUDIO_LIGHTING_PS;
	ec->iHLSLRegister = -1;
	List.hList_EConstants.AddToTail( ec );

	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_NUM_LIGHTS ) );
}








void CHLSL_Solver_StdPSpecLight::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	char tmp[MAXTARGC];
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context,true);

	CHLSL_Var *wpos = GetSourceVar( 0 );
	CHLSL_Var *wnormal = GetSourceVar( 1 );
	CHLSL_Var *eyedir = GetSourceVar( 2 );
	CHLSL_Var *specexp = GetSourceVar( 3 );
	CHLSL_Var *fresnel = GetSourceVar( 4 );
	CHLSL_Var *vattn = GetSourceVar( 5 );
	CHLSL_Var *aoterm = m_bAOTerm ? GetSourceVar( 6 ) : NULL;


	Q_snprintf( tmp, MAXTARGC, "PixelShaderDoSpecularLighting( %s, %s, %s, %s,\n\t\t%s, NUM_LIGHTS, g_cLightInfo,\n\t\t%s, %s, %s, %s );\n",
			wpos->GetName(), wnormal->GetName(), specexp->GetName(), eyedir->GetName(),
			vattn->GetName(),
			( aoterm ? "true" : "false" ),
			( aoterm ? aoterm->GetName() : "1.0f" ),
			fresnel->GetName(),
			tg->GetName()
		);

	context.buf_code.PutString( tmp );

	/*
void PixelShaderDoSpecularLighting( const float3 worldPos, const float3 worldNormal, const float fSpecularExponent, const float3 vEyeDir,
									const float4 lightAtten, const int nNumLights, PixelShaderLightInfo cLightInfo[3],
									const bool bDoAmbientOcclusion, const float fAmbientOcclusion,
									float fFresnel,

									out float3 specularLighting )
	*/
}
void CHLSL_Solver_StdPSpecLight::SetState( bool doAO )
{
	m_bAOTerm = doAO;
}
void CHLSL_Solver_StdPSpecLight::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_UTILITY_ASSIGN, 1, 1 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_UTILITY_ASSIGN ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}
void CHLSL_Solver_StdPSpecLight::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_STUDIO_LIGHTING_PS;
	ec->iHLSLRegister = -1;
	List.hList_EConstants.AddToTail( ec );

	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_NUM_LIGHTS ) );
}