
#include "cbase.h"
#include "editorCommon.h"


CNodeDot::CNodeDot( CNodeView *p ) : BaseClass( "Dotproduct", p )
{
	GenerateJacks_Input( 2 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Input( 1, HLSLJACKFLAGS_F1_TO_F4 );
	//SetJackFlags_Output( 0, HLSLJACKFLAGS_F1_TO_F4 );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT1 );

	GetJack_In( 0 )->SetName( "A" );
	GetJack_In( 1 )->SetName( "B" );
}

CNodeDot::~CNodeDot()
{
}

int CNodeDot::UpdateInputsValid()
{
	int baseLevel = BaseClass::UpdateInputsValid();
	int locallevel = ERRORLEVEL_NONE;

	int vartype_jack_0 = GetJack_In( 0 )->GetSmartType();
	int vartype_jack_1 = GetJack_In( 1 )->GetSmartType();

	int autoTest = TestJackFlags_In();

	if ( autoTest == ERRORLEVEL_NONE )
	{
		if ( vartype_jack_0 != vartype_jack_1 )
		{
			bool bOneIsF1 = ( vartype_jack_0 == HLSLVAR_FLOAT1 ) ||
							( vartype_jack_1 == HLSLVAR_FLOAT1 );
			if ( !bOneIsF1 )
				locallevel = ERRORLEVEL_FAIL;
		}
		else if ( vartype_jack_0 >= HLSLVAR_MATRIX3X3 ||
			vartype_jack_1 >= HLSLVAR_MATRIX3X3 )
				locallevel = ERRORLEVEL_FAIL;
	}

	return max( locallevel, baseLevel );
}
void CNodeDot::UpdateOutputs()
{
	//if ( !GetNumJacks_Out() || !GetNumJacks_In() )
	//	return;

	//if ( GetErrorLevel() != ERRORLEVEL_NONE )
	//	return SetOutputsUndefined();

	//GetJack_Out( 0 )->SetSmartType( max( GetJack_In(0)->GetSmartType(), GetJack_In(1)->GetSmartType() ) );
}
bool CNodeDot::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 2 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ2 = GetJack_In( 1 );
	CJack *pJ_Out = GetJack_Out( 0 );

	int type0 = pJ_Out->GetSmartType();
	const int res = pJ_Out->GetResourceType();

	if ( pJ2->GetSmartType() == HLSLVAR_FLOAT1 )
		swap( pJ1, pJ2 );

	CHLSL_Var *tg = GetInputToWriteTo( type0 );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Dot *solver = new CHLSL_Solver_Dot( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ2->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	//Msg("add solver has %i src vars\n", solver->GetNumSourceVars());

	return true;
}