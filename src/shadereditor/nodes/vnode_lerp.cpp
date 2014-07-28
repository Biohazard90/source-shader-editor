
#include "cbase.h"
#include "editorCommon.h"


CNodeLerp::CNodeLerp( CNodeView *p ) : BaseClass( "Lerp", p )
{
	GenerateJacks_Input( 3 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Input( 1, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Input( 2, HLSLJACKFLAGS_F1_TO_F4 );
	//LockJackInput_Flags( 2, HLSLVAR_FLOAT1, "frac" );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_F1_TO_F4 );

	GetJack_In( 0 )->SetName( "A" );
	GetJack_In( 1 )->SetName( "B" );
	GetJack_In( 2 )->SetName( "frac" );
}

CNodeLerp::~CNodeLerp()
{
}

int CNodeLerp::UpdateInputsValid()
{
	int baseLevel = BaseClass::UpdateInputsValid();
	int locallevel = ERRORLEVEL_NONE;

	int vartype_jack_0 = GetJack_In( 0 )->GetSmartType();
	int vartype_jack_1 = GetJack_In( 1 )->GetSmartType();
	int vartype_jack_2 = GetJack_In( 2 )->GetSmartType();

	int autoTest = TestJackFlags_In();

	if ( autoTest == ERRORLEVEL_NONE )
	{
		if ( vartype_jack_2 > vartype_jack_0 &&
			vartype_jack_2 > vartype_jack_1 )
			locallevel = ERRORLEVEL_FAIL;
		else if ( vartype_jack_2 != vartype_jack_0 &&
			vartype_jack_2 != vartype_jack_1 && 
			vartype_jack_2 != HLSLVAR_FLOAT1 )
			locallevel = ERRORLEVEL_FAIL;

		if ( vartype_jack_0 != vartype_jack_1 )
		{
			bool bOneIsF1 = ( vartype_jack_0 == HLSLVAR_FLOAT1 ) ||
							( vartype_jack_1 == HLSLVAR_FLOAT1 );
			if ( !bOneIsF1 )
				locallevel = ERRORLEVEL_FAIL;
		}
		else if ( vartype_jack_0 >= HLSLVAR_MATRIX3X3 ||
			vartype_jack_1 >= HLSLVAR_MATRIX3X3 || 
			vartype_jack_2 >= HLSLVAR_MATRIX3X3 )
				locallevel = ERRORLEVEL_FAIL;
	}

	return max( locallevel, baseLevel );
}
void CNodeLerp::UpdateOutputs()
{
	if ( !GetNumJacks_Out() || !GetNumJacks_In() )
		return;

	if ( GetErrorLevel() != ERRORLEVEL_NONE )
		return SetOutputsUndefined();

	GetJack_Out( 0 )->SetSmartType( max( GetJack_In(0)->GetSmartType(), GetJack_In(1)->GetSmartType() ) );
}
bool CNodeLerp::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 3 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ2 = GetJack_In( 1 );
	CJack *pJ3 = GetJack_In( 2 );
	CJack *pJ_Out = GetJack_Out( 0 );

	int type1 = pJ1->GetSmartType();
	int type2 = pJ2->GetSmartType();
	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( max( type1, type2 ) );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Lerp *solver = new CHLSL_Solver_Lerp( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ2->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ3->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}