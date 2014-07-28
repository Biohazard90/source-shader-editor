
#include "cbase.h"
#include "editorCommon.h"


CNodeBaseEvaluate::CNodeBaseEvaluate( const char *name, CNodeView *p, int LockOutput, int numInputs, bool bSmall, int LockInput ) : BaseClass( name, p )
{
	if ( bSmall )
	{
		m_flMinSizeX = NODEDEFSIZE_SMALL;
		m_flMinSizeY = NODEDEFSIZE_SMALL;
		m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;
	}
	GenerateJacks_Input( numInputs );
	GenerateJacks_Output( 1 );

	if ( LockOutput > 0 )
		LockJackOutput_Flags( 0, LockOutput );
	else
		SetJackFlags_Output( 0, HLSLJACKFLAGS_F1_TO_F4 );

	HLSLJackVarCluster LockInType = (HLSLJackVarCluster)(( LockInput < 0 ) ? HLSLJACKFLAGS_ALL : LockInput);
	Assert( LockInType >= HLSLJACKFLAGS_F1 && LockInType < HLSLJACKFLAGS_ );

	for ( int i = 0; i < numInputs; i++ )
	{
		SetJackFlags_Input( i, LockInType );
	}

	if ( numInputs == 1 )
		GetJack_In( 0 )->SetName( "In" );
	else
	{
		int start = (int)'A';
		for ( int i = 0; i < numInputs; i++ )
		{
			char tmp[2];
			Q_snprintf( tmp, 2, "%c", start );
			GetJack_In( i )->SetName( tmp );
			start++;
		}
	}
}

CNodeBaseEvaluate::~CNodeBaseEvaluate()
{
}

int CNodeBaseEvaluate::UpdateInputsValid()
{
	int baseLevel = BaseClass::UpdateInputsValid();
	int locallevel = TestJackFlags_In();

	if ( GetNumJacks_In() > 1 && locallevel == ERRORLEVEL_NONE )
	{
		int typeTest = GetJack_In(0)->GetSmartType();
		for ( int i = 1; i < GetNumJacks_In(); i++ )
		{
			int curType = GetJack_In(i)->GetSmartType();
			if ( typeTest != HLSLVAR_FLOAT1 &&
				curType != HLSLVAR_FLOAT1 &&
				typeTest != curType )
				locallevel = ERRORLEVEL_FAIL;

			if ( typeTest > HLSLVAR_FLOAT4 ||
				curType > HLSLVAR_FLOAT4 )
				locallevel = ERRORLEVEL_FAIL;

			if ( typeTest == HLSLVAR_FLOAT1 )
				typeTest = curType;
		}
	}

	return max( locallevel, baseLevel );
}
void CNodeBaseEvaluate::UpdateOutputs()
{
	if ( !GetNumJacks_Out() || !GetNumJacks_In() || GetJack_Out(0)->IsSmartTypeLocked() )
		return;

	if ( GetErrorLevel() != ERRORLEVEL_NONE )
		return SetOutputsUndefined();

	int maxType = GetJack_In(0)->GetSmartType();
	for ( int i = 1; i < GetNumJacks_In(); i++ )
		maxType = max( maxType, GetJack_In(i)->GetSmartType() );
	GetJack_Out( 0 )->SetSmartType( maxType );
}
bool CNodeBaseEvaluate::CreateSolvers(GenericShaderData *ShaderData)
{
	int numIn = GetNumJacks_In();
	if ( GetNumJacks_In_Connected() < numIn )
		return false;

	CJack *pJ_Out = GetJack_Out( 0 );

	int targettype = pJ_Out->GetSmartType();
	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( targettype );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_SolverBase *solver = AllocRangeSolver();
	solver->SetResourceType( res );
	for ( int i = 0; i < numIn; i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}
