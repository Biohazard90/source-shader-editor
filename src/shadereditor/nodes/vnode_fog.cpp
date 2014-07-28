
#include "cbase.h"
#include "editorCommon.h"


CNodeFog::CNodeFog( CNodeView *p ) : BaseClass( "Fog factor", p )
{
#ifdef SHADER_EDITOR_DLL_SWARM
	GenerateJacks_Input( 3 );
#else
	GenerateJacks_Input( 4 );
#endif
	GenerateJacks_Output( 1 );

	LockJackInput_Flags( 0, HLSLVAR_FLOAT4, "Fog params" );
#ifdef SHADER_EDITOR_DLL_SWARM
	LockJackInput_Flags( 1, HLSLVAR_FLOAT3, "View pos" );
	LockJackInput_Flags( 2, HLSLVAR_FLOAT3, "World pos" );
#else
	LockJackInput_Flags( 1, HLSLVAR_FLOAT1, "View pos z" );
	LockJackInput_Flags( 2, HLSLVAR_FLOAT1, "World pos z" );
	LockJackInput_Flags( 3, HLSLVAR_FLOAT1, "Proj pos z" );
#endif

	LockJackOutput_Flags( 0, HLSLVAR_FLOAT1, "Fog factor" );
}

CNodeFog::~CNodeFog()
{
}


bool CNodeFog::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ2 = GetJack_In( 1 );
	CJack *pJ3 = GetJack_In( 2 );
#ifndef SHADER_EDITOR_DLL_SWARM
	CJack *pJ4 = GetJack_In( 3 );
#endif
	CJack *pJ_Out = GetJack_Out( 0 );

	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( pJ_Out->GetSmartType() );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Fog *solver = new CHLSL_Solver_Fog( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ2->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ3->GetTemporaryVarTarget_End() );
#ifndef SHADER_EDITOR_DLL_SWARM
	solver->AddSourceVar( pJ4->GetTemporaryVarTarget_End() );
#endif
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}