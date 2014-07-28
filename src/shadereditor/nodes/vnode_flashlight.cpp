
#include "cbase.h"
#include "editorcommon.h"


CNodeFlashlight::CNodeFlashlight( CNodeView *p ) : BaseClass( "Flashlight", p )
{
	m_bSpecular = false;
	UpdateNode();
}
CNodeFlashlight::~CNodeFlashlight()
{
}

void CNodeFlashlight::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges );

	GenerateJacks_Input( m_bSpecular ? 7 : 4 );
	LockJackInput_Flags( 0, HLSLVAR_FLOAT3, "World pos" );
	LockJackInput_Flags( 1, HLSLVAR_FLOAT3, "World normal" );
	LockJackInput_Flags( 2, HLSLVAR_FLOAT4, "FVP pos" );
	LockJackInput_Flags( 3, HLSLVAR_FLOAT2, "Screen pos" );
	if ( m_bSpecular )
	{
		LockJackInput_Flags( 4, HLSLVAR_FLOAT3, "Eye dir" );
		LockJackInput_Flags( 5, HLSLVAR_FLOAT1, "Spec exp" );
		LockJackInput_Flags( 6, HLSLVAR_FLOAT1, "Fresnel" );
	}

	RestoreBridgesFromList_In( m_hRestoreBridges );



	CreateBridgeRestoreData_Out( CBaseNode::BY_NAME, m_hRestoreBridges );

	GenerateJacks_Output( m_bSpecular ? 2 : 1 );

	LockJackOutput_Flags( 0, HLSLVAR_FLOAT3, "diffuse" );
	if ( m_bSpecular )
		LockJackOutput_Flags( 1, HLSLVAR_FLOAT3, "specular" );

	RestoreBridgesFromList_Out( m_hRestoreBridges );
}

KeyValues *CNodeFlashlight::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_flashlight_Spec", m_bSpecular ? 1 : 0 );
	return pKV;
}
void CNodeFlashlight::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_bSpecular = !!pKV->GetInt( "i_flashlight_Spec", m_bSpecular );
	UpdateNode();
}

bool CNodeFlashlight::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	CJack *pJ_Out_0 = GetJack_Out( 0 );
	CJack *pJ_Out_1 = m_bSpecular ? GetJack_Out( 1 ) : NULL;

	CHLSL_Var *tg_0 = NULL;
	CHLSL_Var *tg_1 = NULL;

	tg_0 = GetInputToWriteTo( pJ_Out_0->GetSmartType() );
	if ( m_bSpecular )
		tg_1 = GetInputToWriteTo( pJ_Out_1->GetSmartType() );

	SetAllocating( !(m_bSpecular ? (tg_0 != NULL) : (tg_0 && tg_1)) );

	if ( !tg_0 )
		tg_0 = pJ_Out_0->AllocateVarFromSmartType();
	pJ_Out_0->SetTemporaryVarTarget( tg_0 );
	if ( m_bSpecular )
	{
		if ( !tg_1 )
			tg_1 = pJ_Out_1->AllocateVarFromSmartType();
		pJ_Out_1->SetTemporaryVarTarget( tg_1 );
	}

	CHLSL_Solver_Flashlight *solver = new CHLSL_Solver_Flashlight( GetUniqueIndex() );
	solver->SetState( m_bSpecular );
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
		solver->AddTargetVar( GetJack_Out(i)->GetTemporaryVarTarget() );
	AddSolver( solver );

	return true;
}


CNodeFlashlight_Position::CNodeFlashlight_Position( CNodeView *p ) : BaseClass( "FL Pos", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT3, "pos" );
}
CNodeFlashlight_Position::~CNodeFlashlight_Position()
{
}
bool CNodeFlashlight_Position::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out_0 = GetJack_Out( 0 );
	CHLSL_Var *tg_0 = pJ_Out_0->AllocateVarFromSmartType();
	tg_0->MakeConstantOnly();

	SetAllocating( false );

	CHLSL_Solver_Flashlight_Pos *solver = new CHLSL_Solver_Flashlight_Pos( GetUniqueIndex() );
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
		solver->AddTargetVar( GetJack_Out(i)->GetTemporaryVarTarget() );
	AddSolver( solver );
	return true;
}