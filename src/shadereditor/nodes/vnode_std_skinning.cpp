
#include "cbase.h"
#include "editorCommon.h"


CNodeStdSkinning::CNodeStdSkinning( CNodeView *p ) : BaseClass( "Skinning", p )
{
	m_iSkinMode = STD_SKINNING_MODE_POS;
	UpdateNode();
}

CNodeStdSkinning::~CNodeStdSkinning()
{
}

void CNodeStdSkinning::SetState( int m )
{
	m_iSkinMode = m;
}

KeyValues *CNodeStdSkinning::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_vskinning_mode", m_iSkinMode );
	return pKV;
}
void CNodeStdSkinning::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_iSkinMode = pKV->GetInt( "i_vskinning_mode", m_iSkinMode );
	UpdateNode();
}

void CNodeStdSkinning::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges );

	int numInputs = 3;
	if ( m_iSkinMode == STD_SKINNING_MODE_POS_NORMAL )
		numInputs = 4;
	else if ( m_iSkinMode == STD_SKINNING_MODE_POS_NORMAL_TANGENT )
		numInputs = 5;
	GenerateJacks_Input( numInputs );

	LockJackInput_Flags( 0, HLSLVAR_FLOAT4, "Bone indices" );
	LockJackInput_Flags( 1, HLSLVAR_FLOAT4, "Bone weights" );
	LockJackInput_Flags( 2, HLSLVAR_FLOAT3, "Obj pos" );
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL )
		LockJackInput_Flags( 3, HLSLVAR_FLOAT3, "Obj normal" );
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL_TANGENT )
		LockJackInput_Flags( 4, HLSLVAR_FLOAT4, "Obj tangent s" );

	RestoreBridgesFromList_In( m_hRestoreBridges );




	CreateBridgeRestoreData_Out( CBaseNode::BY_NAME, m_hRestoreBridges );

	int numOutputs = 1;
	if ( m_iSkinMode == STD_SKINNING_MODE_POS_NORMAL )
		numOutputs = 2;
	else if ( m_iSkinMode == STD_SKINNING_MODE_POS_NORMAL_TANGENT )
		numOutputs = 4;
	GenerateJacks_Output( numOutputs );

	LockJackOutput_Flags( 0, HLSLVAR_FLOAT3, "World pos" );
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL )
		LockJackOutput_Flags( 1, HLSLVAR_FLOAT3, "World normal" );
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL_TANGENT )
	{
		LockJackOutput_Flags( 2, HLSLVAR_FLOAT3, "World tangent s" );
		LockJackOutput_Flags( 3, HLSLVAR_FLOAT3, "World tangent t" );
	}

	RestoreBridgesFromList_Out( m_hRestoreBridges );
}

bool CNodeStdSkinning::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	SetAllocating( true );

	CHLSL_Solver_StdSkinning *solver = new CHLSL_Solver_StdSkinning( GetUniqueIndex() );

	solver->SetState( m_iSkinMode );
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
		solver->AddTargetVar( GetJack_Out(i)->AllocateVarFromSmartType() );

	AddSolver( solver );

	return true;
}










CNodeStdMorph::CNodeStdMorph( CNodeView *p ) : BaseClass( "Morph", p )
{
	m_iSkinMode = STD_SKINNING_MODE_POS;
	UpdateNode();
}

CNodeStdMorph::~CNodeStdMorph()
{
}

void CNodeStdMorph::SetState( int m )
{
	m_iSkinMode = m;
}

KeyValues *CNodeStdMorph::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_vskinning_mode", m_iSkinMode );
	return pKV;
}
void CNodeStdMorph::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_iSkinMode = pKV->GetInt( "i_vskinning_mode", m_iSkinMode );
	UpdateNode();
}

void CNodeStdMorph::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges );

	int numInputs = 2;
	if ( m_iSkinMode == STD_SKINNING_MODE_POS_NORMAL )
		numInputs = 4;
	else if ( m_iSkinMode == STD_SKINNING_MODE_POS_NORMAL_TANGENT )
		numInputs = 5;
	GenerateJacks_Input( numInputs );

	LockJackInput_Flags( 0, HLSLVAR_FLOAT3, "Obj pos" );
	LockJackInput_Flags( 1, HLSLVAR_FLOAT3, "Flex delta" );
	//LockJackInput_Flags( 2, HLSLVAR_FLOAT2, "Morph coords" );
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL )
	{
		LockJackInput_Flags( 2, HLSLVAR_FLOAT3, "Obj normal" );
		LockJackInput_Flags( 3, HLSLVAR_FLOAT3, "Flex normal" );
	}
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL_TANGENT )
		LockJackInput_Flags( 4, HLSLVAR_FLOAT3, "Tangent s" );

	RestoreBridgesFromList_In( m_hRestoreBridges );




	CreateBridgeRestoreData_Out( CBaseNode::BY_NAME, m_hRestoreBridges );

	int numOutputs = 1;
	if ( m_iSkinMode == STD_SKINNING_MODE_POS_NORMAL )
		numOutputs = 2;
	else if ( m_iSkinMode == STD_SKINNING_MODE_POS_NORMAL_TANGENT )
		numOutputs = 3;
	GenerateJacks_Output( numOutputs );

	LockJackOutput_Flags( 0, HLSLVAR_FLOAT3, "Obj pos" );
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL )
		LockJackOutput_Flags( 1, HLSLVAR_FLOAT3, "Obj normal" );
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL_TANGENT )
		LockJackOutput_Flags( 2, HLSLVAR_FLOAT3, "Obj tangent s" );

	RestoreBridgesFromList_Out( m_hRestoreBridges );
}

bool CNodeStdMorph::CreateSolvers(GenericShaderData *ShaderData)
{
	const int numJacksIn = GetNumJacks_In();
	if ( GetNumJacks_In_Connected() < numJacksIn )
		return false;

#define __ALLOCVARONOVERWRITEFAIL( _in, _out ) CHLSL_Var *tg##_out = NULL;\
	if ( _in + 1 > GetNumJacks_In() || _out + 1 > GetNumJacks_Out() )\
	{}\
	else if ( GetJack_In( _in )->GetTemporaryVarTarget_End()->CanBeOverwritten() )\
	{\
		tg##_out = GetJack_In( _in )->GetTemporaryVarTarget_End();\
		GetJack_Out( _out )->SetTemporaryVarTarget( tg##_out );\
	}\
	else\
	{\
		tg##_out = GetJack_Out( _out )->AllocateVarFromSmartType();\
	}

	bool bWillAlloc = !GetJack_In( 0 )->GetTemporaryVarTarget_End()->CanBeOverwritten();
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL && !GetJack_In( 2 )->GetTemporaryVarTarget_End()->CanBeOverwritten() )
		bWillAlloc = true;
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL_TANGENT && !GetJack_In( 4 )->GetTemporaryVarTarget_End()->CanBeOverwritten() )
		bWillAlloc = true;

	SetAllocating( bWillAlloc );

	CHLSL_Solver_StdMorph *solver = new CHLSL_Solver_StdMorph( GetUniqueIndex() );

	solver->SetState( m_iSkinMode );
	for ( int i = 0; i < numJacksIn; i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );

	__ALLOCVARONOVERWRITEFAIL( 0, 0 )
	__ALLOCVARONOVERWRITEFAIL( 2, 1 )
	__ALLOCVARONOVERWRITEFAIL( 4, 2 )

	solver->AddTargetVar( tg0 );
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL )
		solver->AddTargetVar( tg1 );
	if ( m_iSkinMode >= STD_SKINNING_MODE_POS_NORMAL_TANGENT )
		solver->AddTargetVar( tg2 );

	AddSolver( solver );

	return true;
}