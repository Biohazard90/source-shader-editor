
#include "cbase.h"
#include "editorCommon.h"

CNodeVCompression::CNodeVCompression( CNodeView *p ) : BaseClass( "Vertex decompress", p )
{
	m_iCompressionMode = COMPRESSION_MODE_NORMAL;
	UpdateNode();
}

CNodeVCompression::~CNodeVCompression()
{
}

void CNodeVCompression::UpdateNode()
{
	bool bUseTangent = m_iCompressionMode == COMPRESSION_MODE_NORMAL_TANGENT;

	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges );

	GenerateJacks_Input( bUseTangent ? 2 : 1 );
	LockJackInput_Flags( 0, HLSLVAR_FLOAT4, "Normal" );
	if ( bUseTangent )
		LockJackInput_Flags( 1, HLSLVAR_FLOAT4, "Tangent" );

	RestoreBridgesFromList_In( m_hRestoreBridges );


	CreateBridgeRestoreData_Out( CBaseNode::BY_NAME, m_hRestoreBridges );

	GenerateJacks_Output( bUseTangent ? 2 : 1 );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT3, "Normal" );
	if ( bUseTangent )
		LockJackOutput_Flags( 1, HLSLVAR_FLOAT4, "Tangent" );

	RestoreBridgesFromList_Out( m_hRestoreBridges );

	OnUpdateHierachy( NULL, NULL );
}

KeyValues *CNodeVCompression::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "iCompMode", m_iCompressionMode );
	return pKV;
}

void CNodeVCompression::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_iCompressionMode = pKV->GetInt( "iCompMode" );
	UpdateNode();
}

bool CNodeVCompression::CreateSolvers(GenericShaderData *ShaderData)
{
	const bool bUseTangent = m_iCompressionMode == COMPRESSION_MODE_NORMAL_TANGENT;

	SetAllocating( true );
	CHLSL_Solver_VCompression *solver =  new CHLSL_Solver_VCompression( GetUniqueIndex() );
	solver->SetCompressionType( m_iCompressionMode );

	solver->AddSourceVar( GetJack_In(0)->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( GetJack_Out(0)->AllocateVarFromSmartType() );

	if ( bUseTangent )
	{
		solver->AddSourceVar( GetJack_In(1)->GetTemporaryVarTarget_End() );
		solver->AddTargetVar( GetJack_Out(1)->AllocateVarFromSmartType() );
	}

	AddSolver( solver );
	return true;
}