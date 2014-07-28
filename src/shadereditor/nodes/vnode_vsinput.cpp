
#include "cbase.h"
#include "editorCommon.h"
#include "vSheets.h"


CNodeVSInput::CNodeVSInput( CNodeView *p ) : BaseClass( "VS Input", p )
{
	m_flMinSizeX = CRUCIALNODE_MIN_SIZE_X;
	m_flMinSizeY = 0;
	m_bPreviewEnabled = false;

	SETUP_HLSL_VS_Input defaultInfo;
	Setup( defaultInfo );
	//curSetup = defaultInfo;

	//GenerateJacks_Output( 2 );
	//SetupJackOutput( 0, HLSLJACKFLAGS_F3, "Position" );
}

CNodeVSInput::~CNodeVSInput()
{
}

void CNodeVSInput::Setup( SETUP_HLSL_VS_Input info )
{
	int numOutputs = 1;
	if ( info.bFlexDelta_enabled )
		numOutputs += 2;
	if ( info.bBlendweights_enabled )
		numOutputs += 2;
	if ( info.bNormal_enabled )
		numOutputs += 1;
	if ( info.bTangentspace_enabled )
		numOutputs += 2;
	numOutputs += info.iTexcoords_num;
	numOutputs += info.iVertexcolor_num;

	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_Out( CBaseNode::BY_NAME, m_hRestoreBridges );

	GenerateJacks_Output( numOutputs );

	int curOutput = 0;

	GetJack_Out( curOutput )->SetResourceType( RESOURCETYPE_POS_3 );
	LockJackOutput_Flags( curOutput, HLSLVAR_FLOAT3, "Position" );curOutput++;

	if ( info.bFlexDelta_enabled )
	{
		GetJack_Out( curOutput )->SetResourceType( RESOURCETYPE_FLEXDELTA );
		LockJackOutput_Flags( curOutput, HLSLVAR_FLOAT3, "Flex offset" );curOutput++;
		GetJack_Out( curOutput )->SetResourceType( RESOURCETYPE_FLEXDELTA_NORMAL );
		LockJackOutput_Flags( curOutput, HLSLVAR_FLOAT3, "Flex normal" );curOutput++;
	}
	if ( info.bBlendweights_enabled )
	{
		GetJack_Out( curOutput )->SetResourceType( RESOURCETYPE_BONE_WEIGHTS );
		LockJackOutput_Flags( curOutput, HLSLVAR_FLOAT4, "Bone weights" );curOutput++;
		GetJack_Out( curOutput )->SetResourceType( RESOURCETYPE_BONE_INDICES );
		LockJackOutput_Flags( curOutput, HLSLVAR_FLOAT4, "Bone indices" );curOutput++;
	}
	if ( info.bNormal_enabled )
	{
		int normalFlag = info.bNormal_enabled_compression ? HLSLVAR_FLOAT4 : HLSLVAR_FLOAT3;
		GetJack_Out( curOutput )->SetResourceType( info.bNormal_enabled_compression ? RESOURCETYPE_NORMAL_4 : RESOURCETYPE_NORMAL );
		LockJackOutput_Flags( curOutput, normalFlag, "Normal" );curOutput++;
	}
	if ( info.bTangentspace_enabled )
	{
		int tangentStype = info.bTangentspace_enabled_skinning ? HLSLVAR_FLOAT4 : HLSLVAR_FLOAT3;
		int tangentSRestype = info.bTangentspace_enabled_skinning ? RESOURCETYPE_TANGENTS_4 : RESOURCETYPE_TANGENTS_3;

		GetJack_Out( curOutput )->SetResourceType( tangentSRestype );
		LockJackOutput_Flags( curOutput, tangentStype, "Tangent S" );curOutput++;
		GetJack_Out( curOutput )->SetResourceType( RESOURCETYPE_TANGENTT );
		LockJackOutput_Flags( curOutput, HLSLVAR_FLOAT3, "Tangent T" );curOutput++;
	}

	for ( int i = 0; i < info.iTexcoords_num; i++ )
	{
		Assert( i < 3 );
		GetJack_Out( curOutput )->SetResourceType( RESOURCETYPE_TEXCOORD_0 + i );
		LockJackOutput_Flags( curOutput, info.iDataTypeFlag_TexCoords[i], VarArgs("TexCoords %i",i) );curOutput++;
	}
	for ( int i = 0; i < info.iVertexcolor_num; i++ )
	{
		Assert( i < 2 );
		GetJack_Out( curOutput )->SetResourceType( RESOURCETYPE_COLOR_0 + i );
		LockJackOutput_Flags( curOutput, info.iDataTypeFlag_Color[i], VarArgs("Color %i",i) );curOutput++;
	}
	curSetup = info;

	RestoreBridgesFromList_Out( m_hRestoreBridges );
}
KeyValues *CNodeVSInput::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetInt( "enable_flex", curSetup.bFlexDelta_enabled );
	pKV->SetInt( "enable_normal", curSetup.bNormal_enabled );
	pKV->SetInt( "enable_normal_compression", curSetup.bNormal_enabled_compression );
	pKV->SetInt( "enable_blendweights", curSetup.bBlendweights_enabled );
	pKV->SetInt( "enable_tangentspace", curSetup.bTangentspace_enabled );
	pKV->SetInt( "enable_tangentspace_skinning", curSetup.bTangentspace_enabled_skinning );

	pKV->SetInt( "numTexcoords", curSetup.iTexcoords_num );
	pKV->SetInt( "numColor", curSetup.iVertexcolor_num );

	for ( int i = 0; i < 3; i++ )
		pKV->SetInt( VarArgs( "dTFlag_Texcoords_%i", i ), curSetup.iDataTypeFlag_TexCoords[i] );

	for ( int i = 0; i < 2; i++ )
		pKV->SetInt( VarArgs( "dTFlag_Color_%i", i ), curSetup.iDataTypeFlag_Color[i] );
	
	return pKV;
}
void CNodeVSInput::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	SETUP_HLSL_VS_Input info;

	info.bFlexDelta_enabled = !!pKV->GetInt( "enable_flex" );
	info.bNormal_enabled = !!pKV->GetInt( "enable_normal" );
	info.bNormal_enabled_compression = !!pKV->GetInt( "enable_normal_compression" );
	info.bBlendweights_enabled = !!pKV->GetInt( "enable_blendweights" );
	info.bTangentspace_enabled = !!pKV->GetInt( "enable_tangentspace" );
	info.bTangentspace_enabled_skinning = !!pKV->GetInt( "enable_tangentspace_skinning" );

	info.iTexcoords_num = pKV->GetInt( "numTexcoords" );
	info.iVertexcolor_num = pKV->GetInt( "numColor" );

	for ( int i = 0; i < 3; i++ )
		info.iDataTypeFlag_TexCoords[i] = pKV->GetInt( VarArgs( "dTFlag_Texcoords_%i", i ), HLSLVAR_FLOAT2 );
	for ( int i = 0; i < 2; i++ )
		info.iDataTypeFlag_Color[i] = pKV->GetInt( VarArgs( "dTFlag_Color_%i", i ), HLSLVAR_FLOAT4 );

	Setup( info );
}
bool CNodeVSInput::CreateSolvers(GenericShaderData *ShaderData)
{
	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		CJack *pJ = GetJack_Out( i );
		if ( !pJ->GetNumBridgesConnected() )
			continue;

		const int res = pJ->GetResourceType();
		CHLSL_Var *var = pJ->AllocateVarFromSmartType();
		var->MakeConstantOnly();

		CHLSL_Solver_ReadSemantics *solver = new CHLSL_Solver_ReadSemantics( GetUniqueIndex() );
		solver->SendVSSetup( curSetup );
		solver->SetResourceType( res );
		solver->AddTargetVar( var );
		AddSolver( solver );
	}

	SmartCreateDummySolver();
	return true;
}
