
#include "cbase.h"
#include "editorCommon.h"



CNodePP_DrawMat::CNodePP_DrawMat( CNodeView *p ) : BaseClass( "Draw material", p )
{
	m_bAllInputsRequired = false;
	m_iSizingMode_Tg = VPSIZINGMODE_FULL_FB;
	m_iSizingMode_Src = VPSIZINGMODE_FULL_FB;
	m_bPushTarget = true;
	Q_memset( &m_offsets, 0, sizeof( vp_offsets_t ) );

	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hOutputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );

	GenerateJacks_Input( 4 );
	LockJackInput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hInputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );
	LockJackInput_Flags( 1, HLSLVAR_PP_MATERIAL, "Material" );
	LockJackInput_Flags( 2, HLSLVAR_PP_RT, "Ref target (optional)" );
	LockJackInput_Flags( 3, HLSLVAR_PP_RT, "Ref source (optional)" );
}

CNodePP_DrawMat::~CNodePP_DrawMat()
{
}

KeyValues *CNodePP_DrawMat::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetInt( "iSizingMode_Tg", m_iSizingMode_Tg );
	pKV->SetInt( "iSizingMode_Src", m_iSizingMode_Src );
	pKV->SetInt( "iPushTg", m_bPushTarget ? 1 : 0 );

	pKV->SetInt( "iSHEETHINT_enable_tg", GetJack_In(2)->GetNumBridgesConnected() );
	pKV->SetInt( "iSHEETHINT_enable_src", GetJack_In(3)->GetNumBridgesConnected() );

	pKV->SetInt( "iDst_x", m_offsets.dst_x );
	pKV->SetInt( "iDst_y", m_offsets.dst_y );
	pKV->SetInt( "iDst_w", m_offsets.dst_w );
	pKV->SetInt( "iDst_h", m_offsets.dst_h );

	pKV->SetFloat( "flSrcUV_x0", m_offsets.src_x0 );
	pKV->SetFloat( "flSrcUV_y0", m_offsets.src_y0 );
	pKV->SetFloat( "flSrcUV_x", m_offsets.src_x1 );
	pKV->SetFloat( "flSrcUV_y", m_offsets.src_y1 );

	pKV->SetInt( "iSrc_w", m_offsets.src_w );
	pKV->SetInt( "iSrc_h", m_offsets.src_h );

	return pKV;
}

void CNodePP_DrawMat::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_iSizingMode_Tg = pKV->GetInt( "iSizingMode_Tg" );
	m_iSizingMode_Src = pKV->GetInt( "iSizingMode_Src" );
	m_bPushTarget = !!pKV->GetInt( "iPushTg" );

	m_offsets.dst_x = pKV->GetInt( "iDst_x" );
	m_offsets.dst_y = pKV->GetInt( "iDst_y" );
	m_offsets.dst_w = pKV->GetInt( "iDst_w" );
	m_offsets.dst_h = pKV->GetInt( "iDst_h" );

	m_offsets.src_x0 = pKV->GetFloat( "flSrcUV_x0" );
	m_offsets.src_y0 = pKV->GetFloat( "flSrcUV_y0" );
	m_offsets.src_x1 = pKV->GetFloat( "flSrcUV_x" );
	m_offsets.src_y1 = pKV->GetFloat( "flSrcUV_y" );

	m_offsets.src_w = pKV->GetInt( "iSrc_w" );
	m_offsets.src_h = pKV->GetInt( "iSrc_h" );
}

int CNodePP_DrawMat::UpdateInputsValid()
{
	int baseerror = BaseClass::UpdateInputsValid();

	int localerror = ( GetJack_In(0)->GetNumBridgesConnected() < 1 ) ? ERRORLEVEL_UNDEFINED : ERRORLEVEL_NONE;
	localerror = max( ( GetJack_In(1)->GetNumBridgesConnected() < 1 ) ? ERRORLEVEL_UNDEFINED : ERRORLEVEL_NONE, localerror );

	return max( baseerror, localerror );
}

bool CNodePP_DrawMat::CreateSolvers(GenericShaderData *ShaderData)
{
	CHLSL_Solver_PP_DrawMat *solver = new CHLSL_Solver_PP_DrawMat( GetUniqueIndex() );

	int sizing_tg = m_iSizingMode_Tg;
	int sizing_src = m_iSizingMode_Src;

	// mat
	solver->AddSourceVar( GetJack_In(1)->GetTemporaryVarTarget_End() );
	// ref tg
	if ( GetJack_In(2)->GetNumBridgesConnected() > 0 )
	{
		solver->AddSourceVar( GetJack_In(2)->GetTemporaryVarTarget_End() );
		sizing_tg = VPSIZINGMODE_USE_RT;
	}
	// ref src
	if ( GetJack_In(3)->GetNumBridgesConnected() > 0 )
	{
		solver->AddSourceVar( GetJack_In(3)->GetTemporaryVarTarget_End() );
		sizing_src = VPSIZINGMODE_USE_RT;
	}

	solver->Init( sizing_tg, sizing_src, m_bPushTarget, m_offsets );

	AddSolver( solver );
	return true;
}