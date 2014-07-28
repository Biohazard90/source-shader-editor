
#include "cbase.h"
#include "editorCommon.h"


CNodeTexTransform::CNodeTexTransform( CNodeView *p ) : BaseClass( "Texture transform", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT2, "UV" );

	bEnable_Center = false;
	bEnable_Rot = true;
	bEnable_Scale = true;
	bEnable_Trans = true;

	UpdateNode();
}

CNodeTexTransform::~CNodeTexTransform()
{
}

void CNodeTexTransform::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_NAME, m_hRestoreBridges );

	int numIn = bEnable_Center + bEnable_Rot + bEnable_Scale + bEnable_Trans + 1;
	GenerateJacks_Input( numIn );

	int curIn = 0;

	LockJackInput_Flags( curIn, HLSLVAR_FLOAT2, "UV" );
	curIn++;
	if ( bEnable_Center )
	{
		LockJackInput_Flags( curIn, HLSLVAR_FLOAT2, "Center" );
		curIn++;
	}
	if ( bEnable_Rot )
	{
		LockJackInput_Flags( curIn, HLSLVAR_FLOAT1, "Rot" );
		curIn++;
	}
	if ( bEnable_Scale )
	{
		LockJackInput_Flags( curIn, HLSLVAR_FLOAT2, "Scale" );
		curIn++;
	}
	if ( bEnable_Trans )
	{
		LockJackInput_Flags( curIn, HLSLVAR_FLOAT2, "Trans" );
		curIn++;
	}

	RestoreBridgesFromList_In( m_hRestoreBridges );
}

KeyValues *CNodeTexTransform::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetInt( "iTexTrans_center", bEnable_Center ? 1 : 0 );
	pKV->SetInt( "iTexTrans_rot", bEnable_Rot ? 1 : 0 );
	pKV->SetInt( "iTexTrans_scale", bEnable_Scale ? 1 : 0 );
	pKV->SetInt( "iTexTrans_trans", bEnable_Trans ? 1 : 0 );

	return pKV;
}
void CNodeTexTransform::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	bEnable_Center = !!pKV->GetInt( "iTexTrans_center", bEnable_Center ? 1 : 0 );
	bEnable_Rot = !!pKV->GetInt( "iTexTrans_rot", bEnable_Rot ? 1 : 0 );
	bEnable_Scale = !!pKV->GetInt( "iTexTrans_scale", bEnable_Scale ? 1 : 0 );
	bEnable_Trans = !!pKV->GetInt( "iTexTrans_trans", bEnable_Trans ? 1 : 0 );

	UpdateNode();
}

int CNodeTexTransform::UpdateInputsValid()
{
	int baseLevel = BaseClass::UpdateInputsValid();
	int autoTest = TestJackFlags_In();
	return max( autoTest, baseLevel );
}
bool CNodeTexTransform::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	CJack *pJ_Out = GetJack_Out( 0 );

	CHLSL_Var *tg = GetInputToWriteTo( pJ_Out->GetSmartType() );
	SetAllocating( !tg );
	if ( !tg )
		tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_TexTransform *solver = new CHLSL_Solver_TexTransform( GetUniqueIndex() );

	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In( i )->GetTemporaryVarTarget_End() );
	solver->AddTargetVar( tg );

	solver->bEnable_Center = bEnable_Center;
	solver->bEnable_Rot = bEnable_Rot;
	solver->bEnable_Scale = bEnable_Scale;
	solver->bEnable_Trans = bEnable_Trans;

	AddSolver( solver );
	return true;
}