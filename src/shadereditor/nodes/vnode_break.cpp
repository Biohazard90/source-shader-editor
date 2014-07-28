
#include "cbase.h"
#include "editorCommon.h"


CNodeBreak::CNodeBreak( CNodeView *p ) : BaseClass( "Break", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = 0;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;
	m_bAllInputsRequired = false;

	GenerateJacks_Input( 1 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_ALL );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_ALL );
}

CNodeBreak::~CNodeBreak()
{
}

void CNodeBreak::UpdateNode()
{
	int iJacksRequired = 1;
	for ( int i = 0; i < max( GetNumJacks_In(), GetNumJacks_Out() ); i++ )
	{
		const bool bJackInValid = i < GetNumJacks_In();
		const bool bJackOutValid = i < GetNumJacks_Out();
		if ( bJackInValid && GetJack_In( i )->GetNumBridgesConnected() )
			iJacksRequired++;
		else if ( bJackOutValid && GetJack_Out( i )->GetNumBridgesConnected() )
			iJacksRequired++;
	}

	//CUtlVector< int >hSmartTypes_Out;
	//for ( int i = 0; i < GetNumJacks_Out(); i++ )
	//	hSmartTypes_Out.AddToTail( GetJack_Out(i)->GetSmartType() );

	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges_In;
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges_Out;
	CreateBridgeRestoreData_In( CBaseNode::BY_COUNT, m_hRestoreBridges_In );
	CreateBridgeRestoreData_Out( CBaseNode::BY_COUNT, m_hRestoreBridges_Out );

	GenerateJacks_Input( iJacksRequired );
	GenerateJacks_Output( iJacksRequired );

	RestoreBridgesFromList_In( m_hRestoreBridges_In );
	RestoreBridgesFromList_Out( m_hRestoreBridges_Out );

	//for ( int i = 0; i < GetNumJacks_Out(); i++ )
	//	GetJack_Out( i )->SetSmartType( hSmartTypes_Out[i] );
	//hSmartTypes_Out.Purge();

	OnUpdateHierachy( NULL, NULL );
}

void CNodeBreak::PreSolverUpdated()
{
	UpdateNode();
}
void CNodeBreak::PostSolverUpdated()
{
}
int CNodeBreak::UpdateInputsValid()
{
	return BaseClass::UpdateInputsValid();
}
void CNodeBreak::UpdateOutputs()
{
	if ( GetNumJacks_Out() != GetNumJacks_In() )
		return SetOutputsUndefined();

	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		if ( GetJack_In( i )->GetNumBridgesConnected() < 1 )
			break;
		int iSmartTypeIn = GetJack_In( i )->GetSmartType();
		if ( iSmartTypeIn < 0 )
			return SetOutputsUndefined();
		GetJack_Out( i )->SetSmartType( iSmartTypeIn );
	}
}
bool CNodeBreak::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_Out() != GetNumJacks_In() )
		return false;

	CHLSL_Solver_Break *solver = new CHLSL_Solver_Break( GetUniqueIndex() );

	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		CJack *pJ_In = GetJack_In( i );
		if ( pJ_In->GetNumBridgesConnected() < 1 )
			break;
		GetJack_Out( i )->SetTemporaryVarTarget( pJ_In->GetTemporaryVarTarget_End() );
	}

	AddSolver( solver );
	return true;
}




CNodeClip::CNodeClip( CNodeView *p ) : BaseClass( "Clip", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = 0;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	GenerateJacks_Input( 1 );
	GenerateJacks_Output( 1 );

	SetJackFlags_Input( 0, HLSLJACKFLAGS_ALL );
	SetJackFlags_Output( 0, HLSLJACKFLAGS_ALL );
}
CNodeClip::~CNodeClip()
{
}
void CNodeClip::UpdateOutputs()
{
	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		if ( GetJack_In( i )->GetNumBridgesConnected() < 1 )
			break;
		int iSmartTypeIn = GetJack_In( i )->GetSmartType();
		if ( iSmartTypeIn < 0 )
			return SetOutputsUndefined();
		GetJack_Out( i )->SetSmartType( iSmartTypeIn );
	}
}
bool CNodeClip::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	CJack *pJ_In_0 = GetJack_In( 0 );
	CJack *pJ_Out_0 = GetJack_Out( 0 );

	CHLSL_Var *pt = pJ_In_0->GetTemporaryVarTarget_End();

	pJ_Out_0->SetTemporaryVarTarget( pt );

	CHLSL_Solver_Clip *solver = new CHLSL_Solver_Clip( GetUniqueIndex() );
	solver->AddSourceVar( pt );
	solver->AddTargetVar( pt );
	AddSolver( solver );
	return true;
}