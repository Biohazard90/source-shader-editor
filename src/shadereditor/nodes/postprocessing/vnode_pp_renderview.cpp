
#include "cbase.h"
#include "editorCommon.h"

CNodePP_RenderView::CNodePP_RenderView( CNodeView *p ) : BaseClass( "Render view", p )
{
	m_bAllInputsRequired = false;
	m_ViewRenderData = new EditorRenderViewCommand_Data();
	m_bSceneDraw = false;

	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hOutputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );

	GenerateJacks_Input( 3 );
	LockJackInput_Flags( 0, HLSLVAR_PP_MASTER, "" );
	m_hInputs[ 0 ]->SetJackColorCode( HLSLJACK_COLOR_PPMASTER );
	LockJackInput_Flags( 1, HLSLVAR_PP_RT, "RT target" );
	LockJackInput_Flags( 2, HLSLVAR_PP_MATERIAL, "Material override (optional)" );

	OnUpdateHierachy(NULL,NULL);
}

CNodePP_RenderView::~CNodePP_RenderView()
{
	delete m_ViewRenderData;
}

KeyValues *CNodePP_RenderView::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	if ( m_ViewRenderData->GetName() != NULL )
		pKV->SetString( "szVRCName", m_ViewRenderData->GetName() );

	pKV->SetInt( "options_bool_amt", m_ViewRenderData->GetNumBool() );
	pKV->SetInt( "options_int_amt", m_ViewRenderData->GetNumInt() );
	pKV->SetInt( "options_float_amt", m_ViewRenderData->GetNumFloat() );
	pKV->SetInt( "options_string_amt", m_ViewRenderData->GetNumString() );

	for ( int i = 0; i < m_ViewRenderData->GetNumBool(); i++ )
		pKV->SetInt( VarArgs( "options_bool_%i", i ), m_ViewRenderData->GetBoolVal( i ) ? 1 : 0 );

	for ( int i = 0; i < m_ViewRenderData->GetNumInt(); i++ )
		pKV->SetInt( VarArgs( "options_int_%i", i ), m_ViewRenderData->GetIntVal( i ) );

	for ( int i = 0; i < m_ViewRenderData->GetNumFloat(); i++ )
		pKV->SetFloat( VarArgs( "options_float_%i", i ), m_ViewRenderData->GetFloatVal( i ) );

	for ( int i = 0; i < m_ViewRenderData->GetNumString(); i++ )
		pKV->SetString( VarArgs( "options_string_%i", i ), m_ViewRenderData->GetStringVal( i ) );

	pKV->SetInt( "iSceneDraw", m_bSceneDraw ? 1 : 0 );

	return pKV;
}

void CNodePP_RenderView::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_ViewRenderData->ClearAllValues();

	m_ViewRenderData->SetName( pKV->GetString( "szVRCName" ) );

	for ( int i = 0; i < pKV->GetInt( "options_bool_amt" ); i++ )
		m_ViewRenderData->AddBoolValue( !!pKV->GetInt( VarArgs( "options_bool_%i", i ) ) );

	for ( int i = 0; i < pKV->GetInt( "options_int_amt" ); i++ )
		m_ViewRenderData->AddIntValue( pKV->GetInt( VarArgs( "options_int_%i", i ) ) );

	for ( int i = 0; i < pKV->GetInt( "options_float_amt" ); i++ )
		m_ViewRenderData->AddFloatValue( pKV->GetFloat( VarArgs( "options_float_%i", i ) ) );

	for ( int i = 0; i < pKV->GetInt( "options_string_amt" ); i++ )
		m_ViewRenderData->AddStringValue( pKV->GetString( VarArgs( "options_string_%i", i ) ) );

	m_bSceneDraw = !!pKV->GetInt( "iSceneDraw" );

	OnUpdateHierachy(NULL,NULL);
}

int CNodePP_RenderView::UpdateInputsValid()
{
	const int baseerror = BaseClass::UpdateInputsValid();

	int localerror = ( GetJack_In(0)->GetNumBridgesConnected() < 1 ) ? ERRORLEVEL_UNDEFINED : ERRORLEVEL_NONE;
	localerror = max( localerror, ( GetJack_In(1)->GetNumBridgesConnected() < 1 ) ? ERRORLEVEL_UNDEFINED : ERRORLEVEL_NONE );
	localerror = max( localerror,
						(m_ViewRenderData->GetName() == NULL || GetPPCache()->FindVrCallback( m_ViewRenderData->GetName() ) < 0) ?
						ERRORLEVEL_UNDEFINED : ERRORLEVEL_NONE );

	return max( localerror, baseerror );
}

bool CNodePP_RenderView::CreateSolvers(GenericShaderData *ShaderData)
{
	//if ( Q_strlen( m_szRTName ) < 1 )
	//	Assert( 0 );

	//CHLSL_Var *pVar = GetJack_Out(0)->AllocateVarFromSmartType();

	CHLSL_Solver_PP_RenderView *solver = new CHLSL_Solver_PP_RenderView( GetUniqueIndex() );
	solver->Init( m_ViewRenderData, m_bSceneDraw );

	solver->AddSourceVar( GetJack_In(1)->GetTemporaryVarTarget_End() );

	if ( GetJack_In(2)->GetNumBridgesConnected() > 0 )
		solver->AddSourceVar( GetJack_In(2)->GetTemporaryVarTarget_End() );

	AddSolver( solver );
	return true;
}