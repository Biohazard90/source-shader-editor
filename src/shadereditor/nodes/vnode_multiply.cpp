
#include "cbase.h"
#include "editorCommon.h"


CNodeMultiply::CNodeMultiply( CNodeView *p ) : BaseClass( "Multiply", p )
{
	m_bMatrixRotation = false;

	GenerateJacks_Input( 2 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_ALL );
	SetJackFlags_Input( 1, HLSLJACKFLAGS_ALL );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_ALL );

	GetJack_In( 0 )->SetName( "A" );
	GetJack_In( 1 )->SetName( "B" );
}

CNodeMultiply::~CNodeMultiply()
{
}

KeyValues *CNodeMultiply::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_mat_rotation", m_bMatrixRotation ? 1 : 0 );
	return pKV;
}
void CNodeMultiply::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_bMatrixRotation = pKV->GetInt( "i_mat_rotation" ) != 0;

	OnUpdateHierachy( NULL, NULL );
}

int CNodeMultiply::UpdateInputsValid()
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
			{
				// unequal types && none is float1
				// 3 * 3x3 -> float3
				// 3 * 4x3 - append 1 -> float3
				// 4 * 4x3 -> float3
				// 3 * 4x4 - append 1 -> float4
				// 4 * 4x4 -> float4
				if ( vartype_jack_0 != HLSLVAR_FLOAT3 &&
					vartype_jack_0 != HLSLVAR_FLOAT4 &&
					vartype_jack_0 != HLSLVAR_MATRIX3X3 &&
					vartype_jack_0 != HLSLVAR_MATRIX4X3 )
					locallevel = ERRORLEVEL_FAIL;
				else if ( vartype_jack_0 == HLSLVAR_FLOAT3 &&
					vartype_jack_1 != HLSLVAR_MATRIX3X3 &&
					vartype_jack_1 != HLSLVAR_MATRIX4X3 &&
					vartype_jack_1 != HLSLVAR_MATRIX4X4 )
					locallevel = ERRORLEVEL_FAIL;
				else if ( vartype_jack_0 == HLSLVAR_FLOAT4 &&
					vartype_jack_1 != HLSLVAR_MATRIX4X3 &&
					vartype_jack_1 != HLSLVAR_MATRIX4X4 )
					locallevel = ERRORLEVEL_FAIL;
				else if ( vartype_jack_0 == HLSLVAR_MATRIX3X3 &&
					vartype_jack_1 != HLSLVAR_FLOAT3 )
					locallevel = ERRORLEVEL_FAIL;
			}
		}
		else
		{
			// equal types:
			// 1 * 1
			// 2 * 2
			// 3 * 3
			// 4 * 4
			// 3x3 * 3x3
			// 4x4 * 4x4
			if ( vartype_jack_0 == HLSLVAR_MATRIX4X3 )
				locallevel = ERRORLEVEL_FAIL;
		}
	}

	return max( locallevel, baseLevel );
}
void CNodeMultiply::UpdateOutputs()
{
	if ( !GetNumJacks_Out() || !GetNumJacks_In() )
		return;

	if ( GetErrorLevel() != ERRORLEVEL_NONE )
		return SetOutputsUndefined();

	CJack *pJO = GetJack_Out( 0 );
	int vartype_jack_0 = GetJack_In( 0 )->GetSmartType();
	int vartype_jack_1 = GetJack_In( 1 )->GetSmartType();

	int iGoalSmarttype = HLSLVAR_FLOAT4;

	// matrices out
	if ( vartype_jack_0 == HLSLVAR_MATRIX3X3 && vartype_jack_1 == HLSLVAR_MATRIX3X3 )
		iGoalSmarttype = HLSLVAR_MATRIX3X3;
	else if ( vartype_jack_0 == HLSLVAR_MATRIX4X4 && vartype_jack_1 == HLSLVAR_MATRIX4X4 )
		iGoalSmarttype = HLSLVAR_MATRIX4X4;
	else if ( vartype_jack_0 == HLSLVAR_MATRIX4X3 && vartype_jack_1 == HLSLVAR_MATRIX4X4 )
		iGoalSmarttype = HLSLVAR_MATRIX4X4;
	// vector out
	else if ( vartype_jack_0 == HLSLVAR_FLOAT1 || vartype_jack_1 == HLSLVAR_FLOAT1 )
		iGoalSmarttype =  max( vartype_jack_0, vartype_jack_1 );
	else if ( vartype_jack_0 == vartype_jack_1 )
		iGoalSmarttype = vartype_jack_0;
	// vector transform out
	else if ( vartype_jack_1 == HLSLVAR_MATRIX3X3 ||
		vartype_jack_0 == HLSLVAR_MATRIX3X3 ||
		vartype_jack_1 == HLSLVAR_MATRIX4X3 ||
		( vartype_jack_1 == HLSLVAR_MATRIX4X4 && m_bMatrixRotation ) )
		iGoalSmarttype = HLSLVAR_FLOAT3;

	return pJO->SetSmartType( iGoalSmarttype );
	//GetJack_Out( 0 )->SetSmartType( max( GetJack_In(0)->GetSmartType(), GetJack_In(1)->GetSmartType() ) );
}
bool CNodeMultiply::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 2 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ2 = GetJack_In( 1 );
	CJack *pJ_Out = GetJack_Out( 0 );

	int type1 = pJ1->GetSmartType();
	int type2 = pJ2->GetSmartType();
	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = NULL;
	
	if ( type1 == type2 || type1 == HLSLVAR_FLOAT1 || type2 == HLSLVAR_FLOAT1 )
		tg = GetInputToWriteTo( max( type1, type2 ) );
	else if ( type1 == HLSLVAR_FLOAT3 && type2 == HLSLVAR_MATRIX3X3 )
		tg = GetInputToWriteTo( HLSLVAR_FLOAT3 );
	else if ( type1 == HLSLVAR_FLOAT3 && type2 == HLSLVAR_MATRIX4X3 )
		tg = GetInputToWriteTo( HLSLVAR_FLOAT3 );
	else if ( type1 == HLSLVAR_FLOAT4 && type2 == HLSLVAR_MATRIX4X4 )
		tg = GetInputToWriteTo( HLSLVAR_FLOAT4 );
	else if ( type1 == HLSLVAR_MATRIX3X3 && type2 == HLSLVAR_FLOAT3 )
		tg = GetInputToWriteTo( HLSLVAR_FLOAT3 );

	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Multiply *solver = new CHLSL_Solver_Multiply( GetUniqueIndex() );
	solver->SetMatrixRotationOnly( m_bMatrixRotation );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddSourceVar( pJ2->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	//Msg("add solver has %i src vars\n", solver->GetNumSourceVars());

	return true;
}