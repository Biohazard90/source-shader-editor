
#include "cbase.h"
#include "editorCommon.h"


CNodeAppend::CNodeAppend( CNodeView *p ) : BaseClass( "Append", p )
{
	GenerateJacks_Input( 2 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Input( 1, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_F1_TO_F4 );

	GetJack_In( 0 )->SetName( "A" );
	GetJack_In( 1 )->SetName( "B" );
}

CNodeAppend::~CNodeAppend()
{
}

int CNodeAppend::UpdateInputsValid()
{
	int baseLevel = BaseClass::UpdateInputsValid();
	int locallevel = TestJackFlags_In();

	if ( locallevel == ERRORLEVEL_NONE )
	{
		int vartype_jack_0 = GetJack_In( 0 )->GetSmartType();
		int vartype_jack_1 = GetJack_In( 1 )->GetSmartType();

		int numComps_0 = ::GetSlotsFromTypeFlag( vartype_jack_0 );
		int numComps_1 = ::GetSlotsFromTypeFlag( vartype_jack_1 );

		if ( (numComps_0 + numComps_1) > 4 )
			locallevel = ERRORLEVEL_FAIL;
	}

	return max( locallevel, baseLevel );
}
void CNodeAppend::UpdateOutputs()
{
	if ( !GetNumJacks_Out() || !GetNumJacks_In() )
		return;

	if ( GetErrorLevel() != ERRORLEVEL_NONE || GetNumJacks_In_Connected() < GetNumJacks_In() )
		return SetOutputsUndefined();

	int numComps_0 = ::GetSlotsFromTypeFlag( GetJack_In( 0 )->GetSmartType() );
	int numComps_1 = ::GetSlotsFromTypeFlag( GetJack_In( 1 )->GetSmartType() );
	int typeO = ::GetTypeFlagFromEnum( numComps_0 + numComps_1 );

	GetJack_Out( 0 )->SetSmartType( typeO );
}
bool CNodeAppend::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 2 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ2 = GetJack_In( 1 );
	CJack *pJ_Out = GetJack_Out( 0 );

	const int res = pJ_Out->GetResourceType();
	CHLSL_Var *tg = NULL; //GetInputToWriteTo( pJ_Out->GetSmartType() );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Append *solver = new CHLSL_Solver_Append( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ2->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}