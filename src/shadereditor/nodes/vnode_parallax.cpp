
#include "cbase.h"
#include "editorCommon.h"


CNodeParallax::CNodeParallax( CNodeView *p ) : BaseClass( "Parallax UV", p )
{
	UpdateNode();
}

CNodeParallax::~CNodeParallax()
{
}

void CNodeParallax::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges_I, m_hRestoreBridges_O;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges_I );
	CreateBridgeRestoreData_Out( CBaseNode::BY_NAME, m_hRestoreBridges_O );

	const bool bDepthTest = m_PSetup.bGetOffset;
	//GenerateJacks_Input( bDepthTest ? 7 : 6 );
	GenerateJacks_Input( 7 );
	GenerateJacks_Output( bDepthTest ? 2 : 1 );

	int iCurJack = 0;

	LockJackInput_Flags( iCurJack, HLSLVAR_FLOAT2, "UV" );iCurJack++;
	LockJackInput_Flags( iCurJack, HLSLVAR_FLOAT3, "View TSpace" );iCurJack++;
	LockJackInput_Flags( iCurJack, HLSLVAR_FLOAT1, "Parallax Amt" );iCurJack++;
	//if ( bDepthTest )
	{
		LockJackInput_Flags( iCurJack, HLSLVAR_FLOAT3, "World pos" );iCurJack++;
	}
	LockJackInput_Flags( iCurJack, HLSLVAR_FLOAT3, "World normal" );iCurJack++;
	LockJackInput_Flags( iCurJack, HLSLVAR_FLOAT3, "Eye dir" );iCurJack++;
	LockJackInput_Flags( iCurJack, HLSLVAR_SAMPLER, "Heightmap" );iCurJack++;

	LockJackOutput_Flags( 0, HLSLVAR_FLOAT2, "UV" );
	if ( bDepthTest )
		LockJackOutput_Flags( 1, HLSLVAR_FLOAT3, "Tan offset" );


	RestoreBridgesFromList_In( m_hRestoreBridges_I );
	RestoreBridgesFromList_Out( m_hRestoreBridges_O );
}

KeyValues *CNodeParallax::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_samples_min", m_PSetup.iSamplesMin );
	pKV->SetInt( "i_samples_max", m_PSetup.iSamplesMax );
	pKV->SetInt( "i_samples_binary", m_PSetup.iBinaryMax );
	pKV->SetInt( "i_dtest", m_PSetup.bGetOffset ? 1 : 0 );
	pKV->SetInt( "i_gradient", m_PSetup.bGradientLookup ? 1 : 0 );
	pKV->SetInt( "i_prlx_mode", m_PSetup.iParallaxMode );
	return pKV;
}
void CNodeParallax::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_PSetup.iSamplesMin = pKV->GetInt( "i_samples_min" );
	m_PSetup.iSamplesMax = pKV->GetInt( "i_samples_max" );
	m_PSetup.iBinaryMax = pKV->GetInt( "i_samples_binary" );
	m_PSetup.bGetOffset = !!pKV->GetInt( "i_dtest" );
	m_PSetup.bGradientLookup = !!pKV->GetInt( "i_gradient" );
	m_PSetup.iParallaxMode = pKV->GetInt( "i_prlx_mode" );

	UpdateNode();
}

bool CNodeParallax::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	CJack *pJ_Out_0 = GetJack_Out( 0 );
	CJack *pJ_Out_1 = m_PSetup.bGetOffset ? GetJack_Out( 1 ) : NULL;

	const int res = pJ_Out_0->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( pJ_Out_0->GetSmartType() );
	CHLSL_Var *tg2 = pJ_Out_1 ? GetInputToWriteTo( pJ_Out_1->GetSmartType() ) : NULL;
	if ( tg2 == tg )
		tg2 = NULL;

	SetAllocating( !tg || (pJ_Out_1 && !tg2) );

	if ( !tg )
		tg = pJ_Out_0->AllocateVarFromSmartType();
	pJ_Out_0->SetTemporaryVarTarget( tg );

	if ( pJ_Out_1 )
	{
		if ( !tg2 )
			tg2 = pJ_Out_1->AllocateVarFromSmartType();
		pJ_Out_1->SetTemporaryVarTarget( tg2 );
	}

	CHLSL_Solver_Parallax *solver = new CHLSL_Solver_Parallax( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->SetState( m_PSetup );
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	if ( pJ_Out_1 )
		solver->AddTargetVar( tg2 );
	else
		solver->AddTargetVar( new CHLSL_Var( HLSLVAR_FLOAT3 ) );
	AddSolver( solver );
	return true;
}



CNodeParallax_StdShadow::CNodeParallax_StdShadow( CNodeView *p ) : BaseClass( "Parallax shadow", p )
{
	GenerateJacks_Input( 7 );
	GenerateJacks_Output( 1 );

	LockJackInput_Flags( 0, HLSLVAR_FLOAT2, "UV" );
	LockJackInput_Flags( 1, HLSLVAR_FLOAT2, "Parallax UV" );
	LockJackInput_Flags( 2, HLSLVAR_FLOAT3, "Light TSpace" );
	LockJackInput_Flags( 3, HLSLVAR_FLOAT1, "Parallax Amt" );
	LockJackInput_Flags( 4, HLSLVAR_FLOAT3, "World pos" );
	LockJackInput_Flags( 5, HLSLVAR_FLOAT1, "Softening" );
	LockJackInput_Flags( 6, HLSLVAR_SAMPLER, "Heightmap" );
	
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT1, "Light" );
}

CNodeParallax_StdShadow::~CNodeParallax_StdShadow()
{
}

bool CNodeParallax_StdShadow::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	CJack *pJ_Out_0 = GetJack_Out( 0 );

	const int res = pJ_Out_0->GetResourceType();

	CHLSL_Var *tg = GetInputToWriteTo( pJ_Out_0->GetSmartType() );

	SetAllocating( !tg );

	if ( !tg )
		tg = pJ_Out_0->AllocateVarFromSmartType();
	pJ_Out_0->SetTemporaryVarTarget( tg );


	CHLSL_Solver_Parallax_StdShadow *solver = new CHLSL_Solver_Parallax_StdShadow( GetUniqueIndex() );
	solver->SetResourceType( res );
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );
	AddSolver( solver );
	return true;
}