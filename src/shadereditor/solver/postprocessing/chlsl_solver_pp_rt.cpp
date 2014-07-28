
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_PP_RT::CHLSL_Solver_PP_RT( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx )
{
	m_szRTName[0] = '\0';
}

void CHLSL_Solver_PP_RT::Init( const char *rtname )
{
	if ( !rtname || !*rtname )
		return;

	Q_snprintf( m_szRTName, sizeof(m_szRTName), "%s", rtname );
}

void CHLSL_Solver_PP_RT::OnExecuteCode( const RunCodeContext &context )
{
	CHLSL_Var *pT = GetTargetVar(0);

	if ( IsErrorTexture( pT->GetTexture() ) )
		pT->SetTexture( materials->FindTexture( m_szRTName, TEXTURE_GROUP_OTHER ) );

	if ( context.IsUsingPreview() )
	{
		context.pRenderContext->PushRenderTargetAndViewport( pT->GetTexture() );
		BlitRTRect( context );
		context.pRenderContext->PopRenderTargetAndViewport();
	}
}
