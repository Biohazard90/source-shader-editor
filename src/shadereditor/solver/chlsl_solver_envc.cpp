
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_EnvC::CHLSL_Solver_EnvC( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx )
{
	m_iEnvCID = 0;
}

void CHLSL_Solver_EnvC::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	target->SetName( EConstant_GetData(m_iEnvCID)->varname, true );
}

void CHLSL_Solver_EnvC::Render( Preview2DContext &c )
{
	float info_0[4] = { 0, 0, 0, 0 };

	EConstant_GetData( m_iEnvCID )->PreviewSolve( info_0 );

	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( info_0, 4 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_Solver_EnvC::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *c = new SimpleEnvConstant();

	c->iEnvC_ID = m_iEnvCID;
	c->iHLSLRegister = -1;
	c->iSmartNumComps = 1;
	c->flSmartDefaultValues[0] = m_flSmartVal0;

	List.hList_EConstants.AddToTail( c );
}