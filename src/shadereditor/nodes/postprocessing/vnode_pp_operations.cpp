
#include "cbase.h"
#include "editorCommon.h"

CNodePP_ClearBuffers::CNodePP_ClearBuffers( CNodeView *p ) : BaseClass( "Clear buffers", p )
{
	m_bAllInputsRequired = false;

	m_bClearColor = true;
	m_bClearDepth = true;

	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hOutputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );

	GenerateJacks_Input( 2 );
	LockJackInput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hInputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );
	LockJackInput_Flags( 1, HLSLVAR_PP_RT, "RT (optional)" );


	m_iChannelInfo.SetDefaultChannels();
	m_iChannelInfo.bAllowChars = false;
	m_szConstantString[0] = '\0';
}
CNodePP_ClearBuffers::~CNodePP_ClearBuffers()
{
}
int CNodePP_ClearBuffers::UpdateInputsValid()
{
	int baseerror = BaseClass::UpdateInputsValid();

	int localerror = ( GetJack_In(0)->GetNumBridgesConnected() < 1 ) ? ERRORLEVEL_UNDEFINED : ERRORLEVEL_NONE;

	return max( baseerror, localerror );
}
KeyValues *CNodePP_ClearBuffers::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetString( "szConstantString", m_szConstantString );
	pKV->SetInt( "iClear_Color", m_bClearColor ? 1 : 0 );
	pKV->SetInt( "iClear_Depth", m_bClearDepth ? 1 : 0 );

	return pKV;
}
void CNodePP_ClearBuffers::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	char tmp[MAX_PATH];
	Q_snprintf( tmp, sizeof(tmp), "%s", pKV->GetString( "szConstantString" ) );

	m_iChannelInfo.Tokenize( tmp );
	m_iChannelInfo.PrintTargetString( m_szConstantString, MAX_PATH );

	m_bClearColor = !!pKV->GetInt( "iClear_Color" );
	m_bClearDepth = !!pKV->GetInt( "iClear_Depth" );
}
bool CNodePP_ClearBuffers::CreateSolvers(GenericShaderData *ShaderData)
{
	CHLSL_Solver_PP_ClearBuffers *solver = new CHLSL_Solver_PP_ClearBuffers( GetUniqueIndex() );
	solver->Init( m_iChannelInfo, m_bClearColor, m_bClearDepth );

	if ( GetNumJacks_In_Connected() > 1 )
		solver->AddSourceVar( GetJack_In(1)->GetTemporaryVarTarget_End() );

	AddSolver( solver );
	return true;
}


CNodePP_VP_Push::CNodePP_VP_Push( CNodeView *p ) : BaseClass( "Push RT / viewport", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hOutputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );

	GenerateJacks_Input( 2 );
	LockJackInput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hInputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );
	LockJackInput_Flags( 1, HLSLVAR_PP_RT, "RT (optional)" );
}
CNodePP_VP_Push::~CNodePP_VP_Push()
{
}
int CNodePP_VP_Push::UpdateInputsValid()
{
	int pushCount = 1;

	CNodePP_Base *pSearchPop = GetNextPPNode();
	while ( pSearchPop != NULL && pushCount != 0 )
	{
		if ( pSearchPop->GetNodeType() == HLSLNODE_POSTPROCESSING_PUSH_VP )
			pushCount++;

		else if ( pSearchPop->GetNodeType() == HLSLNODE_POSTPROCESSING_POP_VP )
			pushCount--;

		pSearchPop = pSearchPop->GetNextPPNode();
	}

	if ( pushCount != 0 )
		return ERRORLEVEL_FAIL;

	return BaseClass::UpdateInputsValid();
}
bool CNodePP_VP_Push::CreateSolvers(GenericShaderData *ShaderData)
{
	CHLSL_Solver_PP_VP_Push *solver = new CHLSL_Solver_PP_VP_Push( GetUniqueIndex() );
	if ( GetNumJacks_In_Connected() > 1 )
		solver->AddSourceVar( GetJack_In(1)->GetTemporaryVarTarget_End() );
	AddSolver( solver );
	return true;
}



CNodePP_VP_Pop::CNodePP_VP_Pop( CNodeView *p ) : BaseClass( "Pop RT / viewport", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hOutputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );

	GenerateJacks_Input( 1 );
	LockJackInput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hInputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );
}
CNodePP_VP_Pop::~CNodePP_VP_Pop()
{
}
int CNodePP_VP_Pop::UpdateInputsValid()
{
	int pushCount = -1;

	CNodePP_Base *pSearchPop = GetNextPPNode(-1);
	while ( pSearchPop != NULL && pushCount != 0 )
	{
		if ( pSearchPop->GetNodeType() == HLSLNODE_POSTPROCESSING_PUSH_VP )
			pushCount++;

		else if ( pSearchPop->GetNodeType() == HLSLNODE_POSTPROCESSING_POP_VP )
			pushCount--;

		pSearchPop = pSearchPop->GetNextPPNode(-1);
	}

	if ( pushCount != 0 )
		return ERRORLEVEL_FAIL;

	return BaseClass::UpdateInputsValid();
}
bool CNodePP_VP_Pop::CreateSolvers(GenericShaderData *ShaderData)
{
	CHLSL_Solver_PP_VP_Pop *solver = new CHLSL_Solver_PP_VP_Pop( GetUniqueIndex() );
	AddSolver( solver );
	return true;
}




CNodePP_VP_SetRT::CNodePP_VP_SetRT( CNodeView *p ) : BaseClass( "Set RT / viewport", p )
{
	m_bAllInputsRequired = false;

	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hOutputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );

	GenerateJacks_Input( 2 );
	LockJackInput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hInputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );
	LockJackInput_Flags( 1, HLSLVAR_PP_RT, "RT (optional)" );
}
CNodePP_VP_SetRT::~CNodePP_VP_SetRT()
{
}
int CNodePP_VP_SetRT::UpdateInputsValid()
{
	int baseerror = BaseClass::UpdateInputsValid();

	int localerror = ( GetJack_In(0)->GetNumBridgesConnected() < 1 ) ? ERRORLEVEL_UNDEFINED : ERRORLEVEL_NONE;

	return max( baseerror, localerror );
}
bool CNodePP_VP_SetRT::CreateSolvers(GenericShaderData *ShaderData)
{
	CHLSL_Solver_PP_VP_SetRT *solver = new CHLSL_Solver_PP_VP_SetRT( GetUniqueIndex() );

	if ( GetNumJacks_In_Connected() > 1 )
		solver->AddSourceVar( GetJack_In(1)->GetTemporaryVarTarget_End() );

	AddSolver( solver );
	return true;
}




CNodePP_CopyRT::CNodePP_CopyRT( CNodeView *p ) : BaseClass( "Copy RT", p )
{
	m_bAllInputsRequired = false;

	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hOutputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );

	GenerateJacks_Input( 3 );
	LockJackInput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hInputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );
	LockJackInput_Flags( 1, HLSLVAR_PP_RT, "RT source (optional)" );
	LockJackInput_Flags( 2, HLSLVAR_PP_RT, "RT target" );
}
CNodePP_CopyRT::~CNodePP_CopyRT()
{
}
int CNodePP_CopyRT::UpdateInputsValid()
{
	int baseerror = BaseClass::UpdateInputsValid();

	int localerror = ( GetJack_In(0)->GetNumBridgesConnected() < 1 ) ? ERRORLEVEL_UNDEFINED : ERRORLEVEL_NONE;
	localerror = max( localerror, ( GetJack_In(2)->GetNumBridgesConnected() < 1 ) ? ERRORLEVEL_UNDEFINED : ERRORLEVEL_NONE );

	return max( baseerror, localerror );
}
bool CNodePP_CopyRT::CreateSolvers(GenericShaderData *ShaderData)
{
	CHLSL_Solver_PP_CopyRT *solver = new CHLSL_Solver_PP_CopyRT( GetUniqueIndex() );

	solver->AddSourceVar( GetJack_In(2)->GetTemporaryVarTarget_End() );

	if ( GetJack_In(1)->GetNumBridgesConnected() > 0 )
		solver->AddSourceVar( GetJack_In(1)->GetTemporaryVarTarget_End() );

	AddSolver( solver );
	return true;
}




CNodePP_UpdateFB::CNodePP_UpdateFB( CNodeView *p ) : BaseClass( "Update FB copy", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hOutputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );

	GenerateJacks_Input( 1 );
	LockJackInput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hInputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );
}
CNodePP_UpdateFB::~CNodePP_UpdateFB()
{
}
bool CNodePP_UpdateFB::CreateSolvers(GenericShaderData *ShaderData)
{
	CHLSL_Solver_PP_UpdateFB *solver = new CHLSL_Solver_PP_UpdateFB( GetUniqueIndex() );
	AddSolver( solver );
	return true;
}