
#include "cbase.h"
#include "editorCommon.h"


CNodeSwizzle::CNodeSwizzle( CNodeView *p ) : BaseClass( "Swizzle", p )
{
	GenerateJacks_Input( 1 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_F1_TO_F4 );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_F1_TO_F4 );

	GetJack_In( 0 )->SetName( "In" );

	m_SwizzleInfo.SetDefaultChannels();
	szSwizzle[ 0 ] = '\0';
}

CNodeSwizzle::~CNodeSwizzle()
{
}

void CNodeSwizzle::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_Out( CBaseNode::BY_INDEX, m_hRestoreBridges );

	GenerateJacks_Output( 1 );

	if ( !m_SwizzleInfo.IsActive() )
	{
		SetJackFlags_Input( 0, HLSLJACKFLAGS_F1_TO_F4 );

		GetJack_Out( 0 )->SetSmartTypeLocked( false );
		SetJackFlags_Output( 0, HLSLJACKFLAGS_F1_TO_F4 );
	}
	else
	{
		Assert( m_SwizzleInfo.IsActive() );
		int minComps = GetSwizzleMinComponents();
		SetJackFlags_MinFloatRequirement( GetJack_In(0), minComps );
		LockJackOutput_Flags( 0, GetTypeFlagFromEnum(m_SwizzleInfo.iActiveChannels), szSwizzle );
	}

	// HLSLVAR_FLOAT1 - HLSLVAR_FLOAT4

	RestoreBridgesFromList_Out( m_hRestoreBridges );
}
int CNodeSwizzle::GetSwizzleMinComponents()
{
	Assert( m_SwizzleInfo.IsActive() );
	return m_SwizzleInfo.iChannelDepth;
}
int CNodeSwizzle::GetSwizzleTargetComponents()
{
	return m_SwizzleInfo.iActiveChannels;
}

int CNodeSwizzle::PerNodeErrorLevel()
{
	return BaseClass::PerNodeErrorLevel(); // TestJackFlags_In();
}

KeyValues *CNodeSwizzle::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetString( "swizzleString", szSwizzle );
	return pKV;
}
void CNodeSwizzle::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_SwizzleInfo.Tokenize( pKV->GetString( "swizzleString" ) );
	m_SwizzleInfo.PrintTargetString( szSwizzle, sizeof(szSwizzle) );
	UpdateNode();
}

int CNodeSwizzle::UpdateInputsValid()
{
	int baseLevel = BaseClass::UpdateInputsValid();
	return max( baseLevel, TestJackFlags_In() );
}
void CNodeSwizzle::UpdateOutputs()
{
	if ( !GetNumJacks_Out() || !GetNumJacks_In() )
		return;

	if ( m_SwizzleInfo.IsActive() )
		return GetJack_Out( 0 )->SetSmartType( GetTypeFlagFromEnum(m_SwizzleInfo.iActiveChannels) );
	if ( GetErrorLevel() != ERRORLEVEL_NONE )
		return SetOutputsUndefined();

	GetJack_Out( 0 )->SetSmartType( GetJack_In(0)->GetSmartType() );
}
bool CNodeSwizzle::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < 1 )
		return false;

	CJack *pJ1 = GetJack_In( 0 );
	CJack *pJ_Out = GetJack_Out( 0 );

	int numTargetComponents = GetSwizzleTargetComponents();
	if ( !m_SwizzleInfo.IsActive() )
		numTargetComponents = GetSlotsFromTypeFlag( pJ1->GetSmartType() );

	int typeMin = GetTypeFlagFromEnum( numTargetComponents );
	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( typeMin );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_Swizzle *solver = new CHLSL_Solver_Swizzle( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddSourceVar( pJ1->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	//solver->SetTargetSize( numTargetComponents );
	//for ( int i = 0; i < numTargetComponents; i++ )
	//	solver->SetComponentInfo( i, GetTargetComponentInSlot( i ) );
	solver->SetComponentInfo( m_SwizzleInfo );
	AddSolver( solver );

	return true;
}