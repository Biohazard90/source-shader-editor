
#include "cbase.h"
#include "editorCommon.h"


CNodeBaseRange::CNodeBaseRange( const char *name, CNodeView *p ) : BaseClass( name, p )
{
	GenerateJacks_Input( 3 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Input( 1, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Input( 2, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_F1_TO_F4 );

	GetJack_In( 0 )->SetName( "min" );
	GetJack_In( 1 )->SetName( "max" );
	GetJack_In( 2 )->SetName( "value" );
}

CNodeBaseRange::~CNodeBaseRange()
{
}

int CNodeBaseRange::UpdateInputsValid()
{
	int baseLevel = BaseClass::UpdateInputsValid();
	int locallevel = ERRORLEVEL_NONE;

	int vartype_jack_0 = GetJack_In( 0 )->GetSmartType();
	int vartype_jack_1 = GetJack_In( 1 )->GetSmartType();
	int vartype_jack_2 = GetJack_In( 2 )->GetSmartType();

	int autoTest = TestJackFlags_In();

	if ( autoTest == ERRORLEVEL_NONE )
	{
		if ( vartype_jack_0 != HLSLVAR_FLOAT1 &&
			vartype_jack_0 != vartype_jack_2 )
			locallevel = ERRORLEVEL_FAIL;
		if ( vartype_jack_1 != HLSLVAR_FLOAT1 &&
			vartype_jack_1 != vartype_jack_2 )
			locallevel = ERRORLEVEL_FAIL;
	}

	return max( locallevel, baseLevel );
}
void CNodeBaseRange::UpdateOutputs()
{
	if ( !GetNumJacks_Out() || !GetNumJacks_In() )
		return;

	if ( GetErrorLevel() != ERRORLEVEL_NONE )
		return SetOutputsUndefined();

	GetJack_Out( 0 )->SetSmartType( GetJack_In(2)->GetSmartType() );
}
bool CNodeBaseRange::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 3 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ2 = GetJack_In( 1 );
	CJack *pJ3 = GetJack_In( 2 );
	CJack *pJ_Out = GetJack_Out( 0 );

	int type3 = pJ3->GetSmartType();
	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( type3 );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_SolverBase *solver = AllocRangeSolver();
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ2->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ3->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}


CNodeSmoothstep::CNodeSmoothstep( CNodeView *p ) : BaseClass( "Smooth step", p )
{
}
CHLSL_SolverBase *CNodeSmoothstep::AllocRangeSolver()
{
	return new CHLSL_Solver_Smoothstep( GetUniqueIndex() );
}
CNodeClamp::CNodeClamp( CNodeView *p ) : BaseClass( "Clamp", p )
{
}
CHLSL_SolverBase *CNodeClamp::AllocRangeSolver()
{
	return new CHLSL_Solver_Clamp( GetUniqueIndex() );
}