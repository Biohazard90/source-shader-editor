
#include "cbase.h"
#include "editorcommon.h"


CNodeCondition::CNodeCondition( CNodeView *p ) : BaseClass( "Condition", p )
{
	m_iCondition = ITERATORCOND_EQUAL;

	GenerateJacks_Input( 2 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Input( 1, HLSLJACKFLAGS_F1_TO_F4 );

	GetJack_In( 0 )->SetName( "A" );
	GetJack_In( 1 )->SetName( "B" );
}

CNodeCondition::~CNodeCondition()
{
}

int CNodeCondition::UpdateInputsValid()
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
	}

	return max( locallevel, baseLevel );
}

void CNodeCondition::Solve_ContainerEntered()
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return;

	CHLSL_Solver_ConditionIntro *solver_intro = new CHLSL_Solver_ConditionIntro( GetUniqueIndex() );
	solver_intro->m_iCondition = m_iCondition;
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver_intro->AddSourceVar( GetJack_In( i )->GetTemporaryVarTarget_End() );
	AddSolver( solver_intro );

	CHLSL_Solver_ContainerBasic *solver_dummy = new CHLSL_Solver_ContainerBasic( GetUniqueIndex() );
	AddSolver( solver_dummy );
}
void CNodeCondition::Solve_ContainerLeft()
{
	CHLSL_Solver_LoopOutro *solver_outro = new CHLSL_Solver_LoopOutro( GetUniqueIndex() );
	AddSolver( solver_outro );
}

KeyValues *CNodeCondition::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetInt( "i_combo_condition", m_iCondition );

	return pKV;
}
void CNodeCondition::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	BaseClass::RestoreFromKeyValues_Specific( pKV );

	m_iCondition = pKV->GetInt( "i_combo_condition", m_iCondition );

	pNodeView->MakeSolversDirty();
}