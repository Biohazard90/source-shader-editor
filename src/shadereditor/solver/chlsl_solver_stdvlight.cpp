
#include "cbase.h"
#include "editorCommon.h"

void CHLSL_Solver_StdVLight::SetState( bool supportsStaticL, bool doHL )
{
	m_bStaticLighting = supportsStaticL;
	m_bHalfLambert = doHL;
}


void CHLSL_Solver_StdVLight::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	char tmp[MAXTARGC];
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);

	if ( m_bStaticLighting )
	{
		Q_snprintf( tmp, MAXTARGC, "%s = DoLighting( %s, %s, %s, STATIC_LIGHT, DYNAMIC_LIGHT, %s );\n",
			tg->GetName(),
			GetSourceVar(0)->GetName(),
			GetSourceVar(1)->GetName(),
			GetSourceVar(2)->GetName(),
			(m_bHalfLambert ? "true" : "false") );
	}
	else
	{
		const char *nameSrc0 = GetSourceVar(0)->GetName();
		Q_snprintf( tmp, MAXTARGC, "%s = float4( GetVertexAttenForLight( %s, 0 ),\n\t\tGetVertexAttenForLight( %s, 1 ),\n\t\tGetVertexAttenForLight( %s, 2 ),\n\t\tGetVertexAttenForLight( %s, 3 ) );\n",
			tg->GetName(),
			nameSrc0,nameSrc0,nameSrc0,nameSrc0 );
	}

	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_StdVLight::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_STUDIO_LIGHTING_VS;
	ec->iHLSLRegister = -1;
	List.hList_EConstants.AddToTail( ec );

	if ( !m_bStaticLighting )
		return;

	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_LIGHT_STATIC ) );
	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_LIGHT_DYNAMIC ) );
}