
#include "cbase.h"
#include "editorCommon.h"

CNodeTexSample::CNodeTexSample( CNodeView *p ) : BaseClass( "Texture sample", p )
{
	GenerateJacks_Output( 6 );

	m_hOutputs[ 1 ]->SetJackColorCode( HLSLJACK_COLOR_RGB );
	m_hOutputs[ 2 ]->SetJackColorCode( HLSLJACK_COLOR_R );
	m_hOutputs[ 3 ]->SetJackColorCode( HLSLJACK_COLOR_G );
	m_hOutputs[ 4 ]->SetJackColorCode( HLSLJACK_COLOR_B );
	m_hOutputs[ 5 ]->SetJackColorCode( HLSLJACK_COLOR_A );

	LockJackOutput_Flags( 0, HLSLVAR_FLOAT4, "RGBA" );
	LockJackOutput_Flags( 1, HLSLVAR_FLOAT3, "RGB" );
	LockJackOutput_Flags( 2, HLSLVAR_FLOAT1, "R" );
	LockJackOutput_Flags( 3, HLSLVAR_FLOAT1, "G" );
	LockJackOutput_Flags( 4, HLSLVAR_FLOAT1, "B" );
	LockJackOutput_Flags( 5, HLSLVAR_FLOAT1, "A" );

	szParamName[0] = '\0';
	m_szDemoTexturePath[0] = '\0';
	bSRGBRead = false;
	bIs3DTexture = false;
	iTexType = HLSLTEX_BASETEXTURE;

	iFallbackType = 0;
	m_szFallbackTexturePath[0] = '\0';

	iLookupOverride = TEXSAMP_AUTO;

	UpdateInput();
}

CNodeTexSample::~CNodeTexSample()
{
}

void CNodeTexSample::UpdateInput()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_INDEX, m_hRestoreBridges );

	int numIn = 1;
	const bool bHasDerivative = iLookupOverride != TEXSAMP_AUTO && GetSamplerData_UseDerivative( iLookupOverride );
	int Flag = HLSLVAR_FLOAT2;
	const char *jackname = "UV";

	if ( bIs3DTexture )
	{
		Flag = HLSLVAR_FLOAT3;
		jackname = "Dir";
	}
	if ( iLookupOverride != TEXSAMP_AUTO )
	{
		Flag = ::GetVarTypeFlag( GetSampleData_ComponentSize( iLookupOverride ) );
		jackname = GetSamplerData_JackName( iLookupOverride );
	}
	if ( bHasDerivative )
		numIn = 3;
	
	GenerateJacks_Input( numIn );
	LockJackInput_Flags( 0, Flag, jackname );
	if ( bHasDerivative )
	{
		LockJackInput_Flags( 1, Flag, "ddx" );
		LockJackInput_Flags( 2, Flag, "ddy" );
	}

	RestoreBridgesFromList_In( m_hRestoreBridges );
}

KeyValues *CNodeTexSample::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetString( "szParamname", szParamName );
	pKV->SetInt( "iSRGB", bSRGBRead ? 1 : 0 );
	pKV->SetInt( "iTextureType", iTexType );
	pKV->SetInt( "IsCubemap", bIs3DTexture ? 1: 0 );
	pKV->SetInt( "iLookupoverride", iLookupOverride );
	
	pKV->SetString( "szDemoTexturePath", m_szDemoTexturePath );
	pKV->SetString( "szFallbackTexturePath", m_szFallbackTexturePath );
	pKV->SetInt( "iFallbackMode", iFallbackType );
	return pKV;
}
void CNodeTexSample::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	Q_snprintf( szParamName, MAX_PATH, "%s", pKV->GetString( "szParamname" ) );
	bSRGBRead = !!pKV->GetInt( "iSRGB" );
	iTexType = pKV->GetInt( "iTextureType" );
	bIs3DTexture = !!pKV->GetInt( "IsCubemap" );
	iLookupOverride = pKV->GetInt( "iLookupoverride" );

	Q_StripExtension( pKV->GetString("szDemoTexturePath"), m_szDemoTexturePath, sizeof( m_szDemoTexturePath ) );
	Q_StripExtension( pKV->GetString("szFallbackTexturePath"), m_szFallbackTexturePath, sizeof( m_szFallbackTexturePath ) );

//	Q_snprintf( m_szDemoTexturePath, sizeof( m_szDemoTexturePath ), "%s", pKV->GetString("szDemoTexturePath") );
//	Q_snprintf( m_szFallbackTexturePath, sizeof( m_szFallbackTexturePath ), "%s", pKV->GetString("szFallbackTexturePath") );
	iFallbackType = pKV->GetInt( "iFallbackMode" );

	UpdateInput();
}

int CNodeTexSample::UpdateInputsValid()
{
	int baseLevel = BaseClass::UpdateInputsValid();
	int autoTest = TestJackFlags_In();

	if ( Q_strlen( szParamName ) < 1 && iTexType == HLSLTEX_CUSTOMPARAM )
		autoTest = ERRORLEVEL_FAIL;

	return max( baseLevel, autoTest );
}
void CNodeTexSample::UpdateOutputs()
{
}

bool CNodeTexSample::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	if ( GetNumJacks_Out_Connected() )
	{
		for ( int i = 0; i < GetNumJacks_Out(); i++ )
		{
			CJack *pJ_Out = GetJack_Out( i );
			if ( !pJ_Out->GetNumBridgesConnected() )
				continue;

			int mode = i;
			const int res = pJ_Out->GetResourceType();

			CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
			pJ_Out->SetTemporaryVarTarget( tg );
			SetAllocating( true );

			CHLSL_Solver_TextureSample *solver = new CHLSL_Solver_TextureSample( GetUniqueIndex() );
			solver->SetCubemap( bIs3DTexture );
			solver->SetLookupMode( mode );
			solver->SetTextureMode( iTexType );
			solver->SetTextureName( m_szDemoTexturePath );
			solver->SetFallbackName( m_szFallbackTexturePath );
			solver->SetParamName( szParamName );
			solver->SetSrgb( bSRGBRead );
			solver->SetFunction( iLookupOverride );

			solver->SetResourceType( res );
			for ( int i = 0; i < GetNumJacks_In(); i++ )
				solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );
			solver->AddTargetVar( tg );
			AddSolver( solver );
		}
	}
	else
	{
		CJack *pJ_Out = GetJack_Out( 0 );
		CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
		pJ_Out->SetTemporaryVarTarget( tg );
		SetAllocating( true );

		const int res = pJ_Out->GetResourceType();

		CHLSL_Solver_TextureSample *solver = new CHLSL_Solver_TextureSample( GetUniqueIndex() );
		solver->SetCubemap( bIs3DTexture );
		solver->SetTextureMode( iTexType );
		solver->SetLookupMode( 0 );
		solver->SetTextureName( m_szDemoTexturePath );
		solver->SetFallbackName( m_szFallbackTexturePath );
		solver->SetParamName( szParamName );
		solver->SetSrgb( bSRGBRead );
		solver->SetFunction( iLookupOverride );

		solver->SetResourceType( res );
		for ( int i = 0; i < GetNumJacks_In(); i++ )
			solver->AddSourceVar( GetJack_In(i)->GetTemporaryVarTarget_End() );
		solver->AddTargetVar( tg );
		AddSolver( solver );
	}

	return true;
}