
#include "cbase.h"
#include "editorCommon.h"


CNodeCross::CNodeCross( CNodeView *p ) : BaseClass( "Crossproduct", p )
{
	GenerateJacks_Input( 2 );
	GenerateJacks_Output( 1 );

	LockJackInput_Flags( 0, HLSLVAR_FLOAT3, "A" );
	LockJackInput_Flags( 1, HLSLVAR_FLOAT3, "B" );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT3 );
}

CNodeCross::~CNodeCross()
{
}

int CNodeCross::UpdateInputsValid()
{
	int baseLevel = BaseClass::UpdateInputsValid();
	int locallevel = TestJackFlags_In();

	return max( locallevel, baseLevel );
}

bool CNodeCross::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 2 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ2 = GetJack_In( 1 );
	CJack *pJ_Out = GetJack_Out( 0 );

	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( HLSLVAR_FLOAT3 );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Cross *solver = new CHLSL_Solver_Cross( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ2->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}