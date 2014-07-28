
#include "cbase.h"
#include "editorCommon.h"


CNodeFinal::CNodeFinal( CNodeView *p ) : BaseClass( "Final output", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT4, "Color" );

	i_tonemaptype = 1;
	b_writedepth = true;

	UpdateNode();
}

CNodeFinal::~CNodeFinal()
{
}

void CNodeFinal::SetState( int t, bool wd )
{
	i_tonemaptype = t;
	b_writedepth = wd;
}

KeyValues *CNodeFinal::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_final_tonemap", i_tonemaptype );
	pKV->SetInt( "i_final_wdepth", b_writedepth ? 1 : 0 );
	return pKV;
}
void CNodeFinal::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	i_tonemaptype = pKV->GetInt( "i_final_tonemap", i_tonemaptype );
	b_writedepth = !!pKV->GetInt( "i_final_wdepth", b_writedepth );
	UpdateNode();
}

void CNodeFinal::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges );

	GenerateJacks_Input( b_writedepth ? 3 : 2 );
	LockJackInput_Flags( 0, HLSLVAR_FLOAT4, "Color" );
	LockJackInput_Flags( 1, HLSLVAR_FLOAT1, "Fog" );
	if ( b_writedepth )
		LockJackInput_Flags( 2, HLSLVAR_FLOAT1, "Proj z" );

	RestoreBridgesFromList_In( m_hRestoreBridges );
}

bool CNodeFinal::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( pJ_Out->GetSmartType() );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Final *solver = new CHLSL_Solver_Final( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->SetState( i_tonemaptype, b_writedepth );
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}