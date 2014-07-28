
#include "cbase.h"
#include "editorCommon.h"

CNodeSampler::CNodeSampler( CNodeView *p ) : BaseClass( "Sampler object", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_SAMPLER, "Sampler" );

	szParamName[0] = '\0';
	m_szDemoTexturePath[0] = '\0';
	bIs3DTexture = false;
	iTexType = HLSLTEX_BASETEXTURE;

	iFallbackType = 0;
	m_szFallbackTexturePath[0] = '\0';
}

CNodeSampler::~CNodeSampler()
{
}

KeyValues *CNodeSampler::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetString( "szParamname", szParamName );
	pKV->SetInt( "iTextureType", iTexType );
	pKV->SetInt( "IsCubemap", bIs3DTexture ? 1: 0 );
	
	pKV->SetString( "szDemoTexturePath", m_szDemoTexturePath );
	pKV->SetString( "szFallbackTexturePath", m_szFallbackTexturePath );
	pKV->SetInt( "iFallbackMode", iFallbackType );
	return pKV;
}
void CNodeSampler::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	Q_snprintf( szParamName, MAX_PATH, "%s", pKV->GetString( "szParamname" ) );
	iTexType = pKV->GetInt( "iTextureType" );
	bIs3DTexture = !!pKV->GetInt( "IsCubemap" );

	Q_StripExtension( pKV->GetString("szDemoTexturePath"), m_szDemoTexturePath, sizeof( m_szDemoTexturePath ) );
	Q_StripExtension( pKV->GetString("szFallbackTexturePath"), m_szFallbackTexturePath, sizeof( m_szFallbackTexturePath ) );

	iFallbackType = pKV->GetInt( "iFallbackMode" );
}

void CNodeSampler::UpdateOutputs()
{
}

bool CNodeSampler::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	tg->MakeConstantOnly();
	pJ_Out->SetTemporaryVarTarget( tg );
	SetAllocating( false );

	const int res = pJ_Out->GetResourceType();

	CHLSL_Solver_TextureSample *solver = new CHLSL_Solver_TextureSample( GetUniqueIndex() );
	solver->MakeSamplerObject();
	solver->SetCubemap( bIs3DTexture );
	solver->SetTextureMode( iTexType );
	solver->SetLookupMode( 0 );
	solver->SetTextureName( m_szDemoTexturePath );
	solver->SetFallbackName( m_szFallbackTexturePath );
	solver->SetParamName( szParamName );
	solver->SetSrgb( false );
	solver->SetFunction( TEXSAMP_AUTO );

	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}