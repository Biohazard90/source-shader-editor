
#include "cbase.h"
#include "editorCommon.h"
#include "vSheets.h"


CNodePSOutput::CNodePSOutput( CNodeView *p ) : BaseClass( "PS Output", p )
{
	m_flMinSizeX = CRUCIALNODE_MIN_SIZE_X;
	m_flMinSizeY = 0;
	m_bPreviewEnabled = false;

	SETUP_HLSL_PS_Output defaultInfo;
	Setup( defaultInfo );
}

CNodePSOutput::~CNodePSOutput()
{
}

void CNodePSOutput::Setup( SETUP_HLSL_PS_Output info )
{
	int numInputs = 0;
	numInputs += info.iNumRTs;
	if ( info.bDepth )
		numInputs += 1;

	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_RESTYPE, m_hRestoreBridges );

	GenerateJacks_Input( numInputs );

	int curInput = 0;

	for ( int i = 0; i < info.iNumRTs; i++ )
	{
		GetJack_In( curInput )->SetResourceType( RESOURCETYPE_COLOR_0 + i );
		LockJackInput_Flags( curInput, HLSLVAR_FLOAT4, VarArgs("Color %i",i) );
		curInput++;
	}
	if ( info.bDepth )
	{
		GetJack_In( curInput )->SetResourceType( RESOURCETYPE_DEPTH );
		LockJackInput_Flags( curInput, HLSLVAR_FLOAT1, "Depth" );
		curInput++;
	}

	curSetup = info;
	RestoreBridgesFromList_In( m_hRestoreBridges );
}
KeyValues *CNodePSOutput::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetInt( "numColors", curSetup.iNumRTs );
	pKV->SetInt( "enable_Depth", ( curSetup.bDepth ? 1 : 0 ) );

	return pKV;
}
void CNodePSOutput::RestoreFromKeyValues( KeyValues *pKV )
{
	BaseClass::RestoreFromKeyValues( pKV );
}
void CNodePSOutput::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	SETUP_HLSL_PS_Output info;

	info.iNumRTs = pKV->GetInt( "numColors" );
	info.bDepth = !!pKV->GetInt( "enable_Depth" );

	Setup( info );
}

bool CNodePSOutput::CreateSolvers(GenericShaderData *ShaderData)
{
	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		CJack *pJ = GetJack_In( i );
		if ( !pJ->GetNumBridgesConnected() )
			continue;

		CJack *sourceJack = pJ->GetBridge( 0 )->GetInputJack();
		CHLSL_Var *src = sourceJack->GetTemporaryVarTarget();
		const int res = pJ->GetResourceType();

		CHLSL_Solver_WriteSemantics *solver = new CHLSL_Solver_WriteSemantics( GetUniqueIndex() );
		solver->SetResourceType( res );
		solver->AddSourceVar( src );

		AddSolver( solver );
	}

	SmartCreateDummySolver();
	return true;
}