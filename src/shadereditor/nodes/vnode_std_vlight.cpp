
#include "cbase.h"
#include "editorCommon.h"


CNodeStdVLight::CNodeStdVLight( CNodeView *p ) : BaseClass( "Vertex lighting", p )
{
	m_bStaticLighting = false;
	m_bHalfLambert = false;

	UpdateNode();
}

CNodeStdVLight::~CNodeStdVLight()
{
}

KeyValues *CNodeStdVLight::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_vlight_static", m_bStaticLighting ? 1 : 0 );
	pKV->SetInt( "i_vlight_hl", m_bHalfLambert ? 1 : 0 );
	return pKV;
}
void CNodeStdVLight::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_bStaticLighting = !!pKV->GetInt( "i_vlight_static", m_bStaticLighting );
	m_bHalfLambert = !!pKV->GetInt( "i_vlight_hl", m_bHalfLambert );
	UpdateNode();
}

void CNodeStdVLight::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges );

	GenerateJacks_Input( m_bStaticLighting ? 3 : 1 );
	if ( m_bStaticLighting )
	{
		LockJackInput_Flags( 0, HLSLVAR_FLOAT3, "World pos" );
		LockJackInput_Flags( 1, HLSLVAR_FLOAT3, "World normal" );
		LockJackInput_Flags( 2, HLSLVAR_FLOAT3, "Static light" );
	}
	else
	{
		LockJackInput_Flags( 0, HLSLVAR_FLOAT3, "World pos" );
	}

	RestoreBridgesFromList_In( m_hRestoreBridges );

	CreateBridgeRestoreData_Out( CBaseNode::BY_NAME, m_hRestoreBridges );

	GenerateJacks_Output( 1 );
	if ( m_bStaticLighting )
		LockJackOutput_Flags( 0, HLSLVAR_FLOAT3, "Lighting" );
	else
		LockJackOutput_Flags( 0, HLSLVAR_FLOAT4, "VAttn" );

	RestoreBridgesFromList_Out( m_hRestoreBridges );
}

bool CNodeStdVLight::CreateSolvers(GenericShaderData *ShaderData)
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

	CHLSL_Solver_StdVLight *solver = new CHLSL_Solver_StdVLight( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->SetState( m_bStaticLighting, m_bHalfLambert );

	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );

	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}