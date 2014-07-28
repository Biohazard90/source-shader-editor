
#include "cbase.h"
#include "editorCommon.h"


CNodeMCompose::CNodeMCompose( CNodeView *p ) : BaseClass( "MCompose", p )
{
	GenerateJacks_Output( 1 );

	iTargetMatrix = HLSLVAR_MATRIX3X3;
	bColumns = false;
	UpdateNode();
}

CNodeMCompose::~CNodeMCompose()
{
}

void CNodeMCompose::SetTargetMatrix( int flag, bool docolumns )
{
	iTargetMatrix = flag;
	bColumns = docolumns;
}

void CNodeMCompose::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges );

	int numInputs = 3;
	if ( iTargetMatrix == HLSLVAR_MATRIX4X4 ||
		(iTargetMatrix == HLSLVAR_MATRIX4X3 && bColumns) )
		numInputs = 4;

	GenerateJacks_Input( numInputs );

	int type = HLSLVAR_FLOAT3;
	if ( iTargetMatrix == HLSLVAR_MATRIX4X4 ||
		(iTargetMatrix == HLSLVAR_MATRIX4X3 && !bColumns) )
		type = HLSLVAR_FLOAT4;

	for ( int i = 0; i < numInputs; i++ )
		LockJackInput_Flags( i, type, VarArgs( "%c", (((int)'A')+i) ) );

	LockJackOutput_Flags( 0, iTargetMatrix );

	RestoreBridgesFromList_In( m_hRestoreBridges );
}

KeyValues *CNodeMCompose::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_mcomp_matrix", iTargetMatrix );
	pKV->SetInt( "i_mcomp_c", bColumns ? 1 : 0 );
	return pKV;
}
void CNodeMCompose::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	BaseClass::RestoreFromKeyValues_Specific( pKV );
	iTargetMatrix = pKV->GetInt( "i_mcomp_matrix", iTargetMatrix );
	bColumns = !!pKV->GetInt( "i_mcomp_c", bColumns );
	UpdateNode();
}

bool CNodeMCompose::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	CJack *pJ_Out = GetJack_Out( 0 );

	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	SetAllocating( true );

	CHLSL_Solver_MComp *solver = new CHLSL_Solver_MComp( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->SetState( iTargetMatrix, bColumns );
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}