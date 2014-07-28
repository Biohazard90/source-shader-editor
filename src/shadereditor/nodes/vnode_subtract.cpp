
#include "cbase.h"
#include "editorCommon.h"


CNodeSubtract::CNodeSubtract( CNodeView *p ) : BaseClass( "Subtract", p )
{
	GenerateJacks_Input( 2 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Input( 1, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_F1_TO_F4 );

	GetJack_In( 0 )->SetName( "A" );
	GetJack_In( 1 )->SetName( "B" );
}

CNodeSubtract::~CNodeSubtract()
{
}

int CNodeSubtract::UpdateInputsValid()
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
void CNodeSubtract::UpdateOutputs()
{
	if ( !GetNumJacks_Out() || !GetNumJacks_In() )
		return;

	if ( GetErrorLevel() != ERRORLEVEL_NONE )
		return SetOutputsUndefined();

	GetJack_Out( 0 )->SetSmartType( max( GetJack_In(0)->GetSmartType(), GetJack_In(1)->GetSmartType() ) );
}
bool CNodeSubtract::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 2 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ2 = GetJack_In( 1 );
	CJack *pJ_Out = GetJack_Out( 0 );

	int type1 = pJ1->GetSmartType();
	int type2 = pJ2->GetSmartType();
	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( max( type1, type2 ) );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Subtract *solver = new CHLSL_Solver_Subtract( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ2->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}







CNodeInvert::CNodeInvert( CNodeView *p ) : BaseClass( "Invert", p )
{
	GenerateJacks_Input( 1 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_F1_TO_F4 );

	GetJack_In( 0 )->SetName( "In" );
}

CNodeInvert::~CNodeInvert()
{
}

int CNodeInvert::UpdateInputsValid()
{
	int baseLevel = BaseClass::UpdateInputsValid();
	int locallevel = TestJackFlags_In();

	return max( locallevel, baseLevel );
}
void CNodeInvert::UpdateOutputs()
{
	if ( !GetNumJacks_Out() || !GetNumJacks_In() )
		return;

	if ( GetErrorLevel() != ERRORLEVEL_NONE )
		return SetOutputsUndefined();

	GetJack_Out( 0 )->SetSmartType( GetJack_In(0)->GetSmartType() );
}
bool CNodeInvert::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 1 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ_Out = GetJack_Out( 0 );

	int type1 = pJ1->GetSmartType();
	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( type1 );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Invert *solver = new CHLSL_Solver_Invert( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}