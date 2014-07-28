
#include "cbase.h"
#include "editorCommon.h"


CNodeStdPLight::CNodeStdPLight( CNodeView *p ) : BaseClass( "PS lighting", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT3, "Light" );

	m_bStaticLighting = false;
	m_bAOTerm = false;
	m_bHalfLambert = false;

	UpdateNode();
}

CNodeStdPLight::~CNodeStdPLight()
{
}

KeyValues *CNodeStdPLight::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_plight_static", m_bStaticLighting ? 1 : 0 );
	pKV->SetInt( "i_plight_ao", m_bAOTerm ? 1 : 0 );
	pKV->SetInt( "i_plight_hl", m_bHalfLambert ? 1 : 0 );
	return pKV;
}
void CNodeStdPLight::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_bStaticLighting = !!pKV->GetInt( "i_plight_static", m_bStaticLighting );
	m_bAOTerm = !!pKV->GetInt( "i_plight_ao", m_bAOTerm );
	m_bHalfLambert = !!pKV->GetInt( "i_plight_hl", m_bHalfLambert );
	UpdateNode();
}

void CNodeStdPLight::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges );

	// pos, normal, attn
	// static col
	// ao
	int numIn = 3;
	if ( m_bStaticLighting )
		numIn += 1;
	if ( m_bAOTerm )
		numIn += 1;

	GenerateJacks_Input( numIn );
	int curIn = 0;
	LockJackInput_Flags( curIn, HLSLVAR_FLOAT3, "World pos" );
	curIn++;
	LockJackInput_Flags( curIn, HLSLVAR_FLOAT3, "World normal" );
	curIn++;
	LockJackInput_Flags( curIn, HLSLVAR_FLOAT4, "Light attn" );
	curIn++;

	if ( m_bStaticLighting )
	{
		LockJackInput_Flags( curIn, HLSLVAR_FLOAT3, "Static light" );
		curIn++;
	}
	if ( m_bAOTerm )
	{
		LockJackInput_Flags( curIn, HLSLVAR_FLOAT1, "AO term" );
	}

	RestoreBridgesFromList_In( m_hRestoreBridges );
}

bool CNodeStdPLight::CreateSolvers(GenericShaderData *ShaderData)
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

	CHLSL_Solver_StdPLight *solver = new CHLSL_Solver_StdPLight( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->SetState( m_bStaticLighting, m_bHalfLambert, m_bAOTerm );

	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );

	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}






CNodeStdPSpecLight::CNodeStdPSpecLight( CNodeView *p ) : BaseClass( "PS specular", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT3, "Spec" );

	m_bAOTerm = false;

	UpdateNode();
}

CNodeStdPSpecLight::~CNodeStdPSpecLight()
{
}

KeyValues *CNodeStdPSpecLight::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_plight_ao", m_bAOTerm ? 1 : 0 );
	return pKV;
}
void CNodeStdPSpecLight::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_bAOTerm = !!pKV->GetInt( "i_plight_ao", m_bAOTerm );
	UpdateNode();
}

void CNodeStdPSpecLight::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges );

	// pos, normal, attn
	// ao
	int numIn = 6;
	if ( m_bAOTerm )
		numIn += 1;

	GenerateJacks_Input( numIn );
	int curIn = 0;
	LockJackInput_Flags( curIn, HLSLVAR_FLOAT3, "World pos" );
	curIn++;
	LockJackInput_Flags( curIn, HLSLVAR_FLOAT3, "World normal" );
	curIn++;
	LockJackInput_Flags( curIn, HLSLVAR_FLOAT3, "Eye dir" );
	curIn++;
	LockJackInput_Flags( curIn, HLSLVAR_FLOAT1, "Spec exp" );
	curIn++;
	LockJackInput_Flags( curIn, HLSLVAR_FLOAT1, "Fresnel" );
	curIn++;
	LockJackInput_Flags( curIn, HLSLVAR_FLOAT4, "Light attn" );
	curIn++;

	if ( m_bAOTerm )
	{
		LockJackInput_Flags( curIn, HLSLVAR_FLOAT1, "AO term" );
	}

	RestoreBridgesFromList_In( m_hRestoreBridges );
}
bool CNodeStdPSpecLight::CreateSolvers(GenericShaderData *ShaderData)
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

	CHLSL_Solver_StdPSpecLight *solver = new CHLSL_Solver_StdPSpecLight( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->SetState( m_bAOTerm );

	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );

	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}