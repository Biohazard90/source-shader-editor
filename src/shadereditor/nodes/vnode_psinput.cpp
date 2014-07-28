
#include "cbase.h"
#include "editorCommon.h"
#include "vSheets.h"


CNodePSInput::CNodePSInput( CNodeView *p ) : BaseClass( "PS Input", p )
{
	m_flMinSizeX = CRUCIALNODE_MIN_SIZE_X;
	m_flMinSizeY = 0;
	m_bPreviewEnabled = false;

	SETUP_HLSL_VS_Output_PS_Input defaultInfo;
	Setup( defaultInfo );
}

CNodePSInput::~CNodePSInput()
{
}

void CNodePSInput::Setup( SETUP_HLSL_VS_Output_PS_Input info )
{
	int numInputs = 0;
	numInputs += info.iTexcoords_num;
	numInputs += info.iVertexcolor_num;

	int extraRows = 0;
	for ( int i = 0; i < info.iTexcoords_num; i++ )
	{
		int rows = max(0,GetVarFlagsRowsRequired( info.iTexCoordFlag[i] ) - 1);
		extraRows += rows;
	}

	numInputs -= extraRows;

	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_Out( CBaseNode::BY_RESTYPE, m_hRestoreBridges );

	GenerateJacks_Output( numInputs );

	int curInput = 0;
	int texCoordNameOffset = 0;
	//int texCoordLookupOffset = 0;

	for ( int i = 0; i < info.iTexcoords_num - extraRows; i++ )
	{
		GetJack_Out( curInput )->SetResourceType( RESOURCETYPE_TEXCOORD_0 + i );
		LockJackOutput_Flags( curInput, info.iTexCoordFlag[texCoordNameOffset], VarArgs("TexCoords %i", texCoordNameOffset) );
		int localRows = GetVarFlagsRowsRequired( info.iTexCoordFlag[ texCoordNameOffset ] );
		texCoordNameOffset += localRows;
		curInput++;
	}

	for ( int i = 0; i < info.iVertexcolor_num; i++ )
	{
		GetJack_Out( curInput )->SetResourceType( RESOURCETYPE_COLOR_0 + i );
		LockJackOutput_Flags( curInput, info.iColorFlag[i], VarArgs("Color %i",i) );
		curInput++;
	}
	curSetup = info;

	RestoreBridgesFromList_Out( m_hRestoreBridges );
}
void CNodePSInput::LoadNames()
{
	int numInputs = 0;
	numInputs += curSetup.iTexcoords_num;
	numInputs += curSetup.iVertexcolor_num;

	int extraRows = 0;
	for ( int i = 0; i < curSetup.iTexcoords_num; i++ )
	{
		int rows = max(0,GetVarFlagsRowsRequired( curSetup.iTexCoordFlag[i] ) - 1);
		extraRows += rows;
	}

	numInputs -= extraRows;

	int curInput = 0;
	int texCoordNameOffset = 0;

	for ( int i = 0; i < curSetup.iTexcoords_num - extraRows; i++ )
	{
		GetJack_Out(curInput)->SetResourceType( RESOURCETYPE_TEXCOORD_0 + texCoordNameOffset );
		if ( Q_strlen( curNames.names_texcoords[ texCoordNameOffset ] ) )
			GetJack_Out( curInput )->SetName( curNames.names_texcoords[ texCoordNameOffset ] );
		int localRows = GetVarFlagsRowsRequired( curSetup.iTexCoordFlag[ texCoordNameOffset ] );
		texCoordNameOffset += localRows;
		curInput++;
	}
	for ( int i = 0; i < curSetup.iVertexcolor_num; i++ )
	{
		if ( Q_strlen( curNames.names_colors[ i ] ) )
			GetJack_Out( curInput )->SetName( curNames.names_colors[ i ] );
		curInput++;
	}
}
KeyValues *CNodePSInput::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetInt( "numTexcoords", curSetup.iTexcoords_num );
	pKV->SetInt( "numColor", curSetup.iVertexcolor_num );
	for ( int i = 0; i < VSTOPS_NUM_TEXCOORDS; i++ )
	{
		pKV->SetInt( VarArgs( "numTexcoord_Flag_%i", i ), curSetup.iTexCoordFlag[ i ] );
		pKV->SetString( VarArgs( "numTexcoord_Name_%i", i ), curNames.names_texcoords[ i ] );
	}
	for ( int i = 0; i < VSTOPS_NUM_COLORS; i++ )
	{
		pKV->SetInt( VarArgs( "numColor_Flag_%i", i ), curSetup.iColorFlag[ i ] );
		pKV->SetString( VarArgs( "numColor_Name_%i", i ), curNames.names_colors[ i ] );
	}
	return pKV;
}
void CNodePSInput::RestoreFromKeyValues( KeyValues *pKV )
{
	BaseClass::RestoreFromKeyValues( pKV );

	CBaseNode *pPartner = pNodeView->GetNodeFromType( HLSLNODE_VS_OUT );
	if ( pPartner )
		pPartner->RestoreFromKeyValues_Specific( pKV );
}
void CNodePSInput::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	SETUP_HLSL_VS_Output_PS_Input info;

	info.iTexcoords_num = pKV->GetInt( "numTexcoords" );
	info.iVertexcolor_num = pKV->GetInt( "numColor" );
	for ( int i = 0; i < VSTOPS_NUM_TEXCOORDS; i++ )
	{
		info.iTexCoordFlag[ i ] = pKV->GetInt( VarArgs( "numTexcoord_Flag_%i", i ) );
		Q_snprintf( curNames.names_texcoords[ i ], sizeof(curNames.names_texcoords[ i ]), "%s", pKV->GetString( VarArgs( "numTexcoord_Name_%i", i ) ) );
	}
	for ( int i = 0; i < VSTOPS_NUM_COLORS; i++ )
	{
		info.iColorFlag[ i ] = pKV->GetInt( VarArgs( "numColor_Flag_%i", i ) );
		Q_snprintf( curNames.names_colors[ i ], sizeof(curNames.names_colors[ i ]), "%s", pKV->GetString( VarArgs( "numColor_Name_%i", i ) ) );
	}

	Setup( info );
	LoadNames();
}
int CNodePSInput::PerNodeErrorLevel()
{
#if PSIN_ERROR_WHEN_VSOUT_ERROR
	CNodeView *pView = GetParent();
	CBaseNode *pVS_Out = pView ? pView->GetNodeFromType( HLSLNODE_VS_OUT ) : NULL;
	if ( !pVS_Out )
		return ERRORLEVEL_NONE;
	if ( pVS_Out->GetErrorLevel() != ERRORLEVEL_NONE )
		return ERRORLEVEL_FAIL;
#endif
	return BaseClass::PerNodeErrorLevel();
}

bool CNodePSInput::CreateSolvers(GenericShaderData *ShaderData)
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
		solver->SetResourceType( res );
		solver->AddTargetVar( var );
		solver->GetData()._IntValue_0 = pJ->GetSmartType();
		//if ( pJ->GetSmartType() < 0 )
		//	Warning("smarttype error!!\n");

		AddSolver( solver );
	}

	SmartCreateDummySolver();
	return true;
}
