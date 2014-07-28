
#include "cbase.h"
#include "editorcommon.h"


CNodeLoop::CNodeLoop( CNodeView *p ) : BaseClass( "Loop", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT1, "Iteration" );

	m_iCondition = ITERATORCOND_SMALLER;
	m_iValue_Start = 0;
	m_iValue_End = 1;
	bDecrement = false;
}

CNodeLoop::~CNodeLoop()
{
}

void CNodeLoop::Solve_ContainerEntered()
{
	CJack *pJO = GetJack_Out( 0 );
	CHLSL_Var *varIterator = pJO->AllocateVarFromSmartType();
	varIterator->MakeConstantOnly();

	CHLSL_Solver_LoopIntro *solver_intro = new CHLSL_Solver_LoopIntro( GetUniqueIndex() );
	CHLSL_Solver_Iterator *solver_iterator = new CHLSL_Solver_Iterator( GetUniqueIndex() );

	solver_intro->m_iCondition = m_iCondition;
	solver_intro->m_iValue_Start = m_iValue_Start;
	solver_intro->m_iValue_End = m_iValue_End;
	solver_intro->bDecrement = bDecrement;
	solver_intro->AddTargetVar( varIterator );
	AddSolver( solver_intro );

	solver_iterator->AddTargetVar( varIterator );
	AddSolver( solver_iterator );
}
void CNodeLoop::Solve_ContainerLeft()
{
	CHLSL_Solver_LoopOutro *solver_outro = new CHLSL_Solver_LoopOutro( GetUniqueIndex() );
	AddSolver( solver_outro );
}

KeyValues *CNodeLoop::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetInt( "loop_condition", m_iCondition );
	pKV->SetInt( "loop_value_min", m_iValue_Start );
	pKV->SetInt( "loop_value_max", m_iValue_End );

	return pKV;
}
void CNodeLoop::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	BaseClass::RestoreFromKeyValues_Specific( pKV );

	m_iCondition = pKV->GetInt( "loop_condition", m_iCondition );
	m_iValue_Start = pKV->GetInt( "loop_value_min", m_iValue_Start );
	m_iValue_End = pKV->GetInt( "loop_value_max", m_iValue_End );

	bDecrement = m_iValue_Start > m_iValue_End;
	pNodeView->MakeSolversDirty();
}