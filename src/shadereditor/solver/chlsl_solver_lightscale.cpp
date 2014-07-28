
#include "cbase.h"
#include "editorCommon.h"

void CHLSL_Solver_Lightscale::SetState( int lscaletype )
{
	m_iLightScale = lscaletype;
}

void CHLSL_Solver_Lightscale::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	target->SetName( GetLightscaleCodeString( m_iLightScale ), true );
}

void CHLSL_Solver_Lightscale::Render( Preview2DContext &c )
{
	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( 1, 1, 1, 1 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_Solver_Lightscale::OnIdentifierAlloc( IdentifierLists_t &List )
{
	if ( m_iLightScale == LSCALE_LIGHTMAP_RGB )
	{
		SimpleEnvConstant *ec = new SimpleEnvConstant();
		ec->iEnvC_ID = HLSLENV_LIGHTMAP_RGB;
		ec->iHLSLRegister = -1;
		ec->iConstSize = 1;
		List.hList_EConstants.AddToTail( ec );
	}
}