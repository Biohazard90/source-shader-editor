
#include "cbase.h"
#include "editorCommon.h"

CNodeUtility_Declare::CNodeUtility_Declare( CNodeView *p ) : BaseClass( "Declare", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	GenerateJacks_Input( 1 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_ALL );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_ALL );
}
CNodeUtility_Declare::~CNodeUtility_Declare()
{
}
int CNodeUtility_Declare::UpdateInputsValid()
{
	return BaseClass::UpdateInputsValid();
}
void CNodeUtility_Declare::UpdateOutputs()
{
	if ( !GetNumJacks_Out() || !GetNumJacks_In() )
		return;

	if ( GetErrorLevel() != ERRORLEVEL_NONE )
		return SetOutputsUndefined();

	GetJack_Out( 0 )->SetSmartType( GetJack_In(0)->GetSmartType() );
}
bool CNodeUtility_Declare::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 1 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ_Out = GetJack_Out( 0 );

	const int res = pJ_Out->GetResourceType();

	SetAllocating( true );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();

	CHLSL_Solver_Utility_Declare *solver = new CHLSL_Solver_Utility_Declare( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}



CNodeUtility_Assign::CNodeUtility_Assign( CNodeView *p ) : BaseClass( "Assign", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	GenerateJacks_Input( 2 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_ALL );
	SetJackFlags_Input( 1, HLSLJACKFLAGS_ALL );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_ALL );
}
CNodeUtility_Assign::~CNodeUtility_Assign()
{
}
int CNodeUtility_Assign::UpdateInputsValid()
{
	int vartype_jack_0 = GetJack_In( 0 )->GetSmartType();
	int vartype_jack_1 = GetJack_In( 1 )->GetSmartType();
	int autoTest = TestJackFlags_In();

	if ( autoTest == ERRORLEVEL_NONE )
	{
		if ( vartype_jack_0 != vartype_jack_1 )
			return ERRORLEVEL_FAIL;
	}

	return max( autoTest, BaseClass::UpdateInputsValid() );
}
void CNodeUtility_Assign::UpdateOutputs()
{
	if ( !GetNumJacks_Out() || !GetNumJacks_In() )
		return;

	if ( GetErrorLevel() != ERRORLEVEL_NONE )
		return SetOutputsUndefined();

	GetJack_Out( 0 )->SetSmartType( GetJack_In(0)->GetSmartType() );
}
bool CNodeUtility_Assign::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 2 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ2 = GetJack_In( 1 );
	CJack *pJ_Out = GetJack_Out( 0 );
	if ( !pJ1->GetTemporaryVarTarget_End()->CanBeOverwritten() )
	{
		SetOutputsUndefined();
		OnShowSolverErrored();
		return false;
	}


	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = pJ1->GetTemporaryVarTarget_End_Smart( pJ1->GetSmartType(), true );
	Assert( tg );
	if ( !tg->CanBeOverwritten() )
		tg = NULL;
	if ( !( tg->GetType() & pJ1->GetSmartType() ) )
		tg = NULL;

	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Utility_Assign *solver = new CHLSL_Solver_Utility_Assign( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ2->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}