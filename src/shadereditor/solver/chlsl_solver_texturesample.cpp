
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_TextureSample::CHLSL_Solver_TextureSample( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx )
{
	m_bIsSamplerObject = false;

	pKV = NULL;
	pRenderMaterial = NULL;

	szCustomTextureName = NULL;
	szCustomParamName = NULL;
	szFallbackName = NULL;

	bSrgb = false;
	lastSourceIndex = -1;
	iTextureMode = 0;
	iLookupMode = 0;

	iFunctionOverride = TEXSAMP_AUTO;
}
CHLSL_Solver_TextureSample::~CHLSL_Solver_TextureSample()
{
	DestroySolverMaterial();

	//if ( pKV )
	//{
	//	delete ((void*)pKV);
	//	pKV = NULL;
	//}
#if DEBUG
	if ( szCustomTextureName )
		AllocCheck_FreeS( "szCustomTextureName" );
#endif

	delete [] szCustomTextureName;
	delete [] szCustomParamName;
	delete [] szFallbackName;
}
CHLSL_Solver_TextureSample::CHLSL_Solver_TextureSample( const CHLSL_Solver_TextureSample& o ) : CHLSL_SolverBase( o )
{
	pRenderMaterial = NULL;
	pKV = NULL;

	m_bIsSamplerObject = o.m_bIsSamplerObject;
	iTextureMode = o.iTextureMode;
	iLookupMode = o.iLookupMode;
	bCubemap = o.bCubemap;
	bSrgb = o.bSrgb;
	lastSourceIndex = -1;
	iFunctionOverride = o.iFunctionOverride;

	AutoCopyStringPtr( o.szCustomTextureName, &szCustomTextureName );
	AutoCopyStringPtr( o.szCustomParamName, &szCustomParamName );
	AutoCopyStringPtr( o.szFallbackName, &szFallbackName );

#if DEBUG
	if ( szCustomTextureName )
		AllocCheck_AllocS( "szCustomTextureName" );
#endif
}

void CHLSL_Solver_TextureSample::MakeSamplerObject()
{
	m_bIsSamplerObject = true;
}

void CHLSL_Solver_TextureSample::SetFallbackName( const char *n )
{
	delete [] szFallbackName;
	AutoCopyStringPtr( n, &szFallbackName );
}
void CHLSL_Solver_TextureSample::SetTextureName( const char *n )
{
#if DEBUG
	if ( szCustomTextureName )
		AllocCheck_FreeS( "szCustomTextureName" );
#endif
	delete [] szCustomTextureName;
	AutoCopyStringPtr( n, &szCustomTextureName );
#if DEBUG
	if ( szCustomTextureName )
		AllocCheck_AllocS( "szCustomTextureName" );
#endif
}
void CHLSL_Solver_TextureSample::SetParamName( const char *n )
{
	delete [] szCustomParamName;
	AutoCopyStringPtr( n, &szCustomParamName );
}

void CHLSL_Solver_TextureSample::SetRenderMaterial( IMaterial *pM )
{
	pRenderMaterial = pM;
}

static char *ReadModeToChar[TEXSAMPLER_LOOKUPMODE_LAST] = {
	"rgba",
	"rgb",
	"r",
	"g",
	"b",
	"a",
};
SimpleTexture *CHLSL_Solver_TextureSample::GetActiveTexture( const WriteContext_FXC &context,
	bool *bHasSolverSiblings, bool *bIsFirstSibling, int *lookupIndex )
{
	CUtlVector< SimpleTexture* > &_texList = context.m_pActive_Identifiers->hList_Textures;
	SimpleTexture *activeTex = NULL;

	if ( bHasSolverSiblings )
		*bHasSolverSiblings = false;
	if ( bIsFirstSibling )
		*bIsFirstSibling = false;
	if ( lookupIndex )
		*lookupIndex = -1;

	for ( int s = 0; s < context.m_hActive_Solvers->Count(); s++ )
	{
		CHLSL_SolverBase *solver = context.m_hActive_Solvers->Element( s );
		if ( solver->GetData().iNodeIndex == GetData().iNodeIndex )
		{
			if ( bIsFirstSibling != NULL && bHasSolverSiblings != NULL &&
				!(*bHasSolverSiblings) && solver == this )
				*bIsFirstSibling = true;
			else if ( bHasSolverSiblings != NULL )
				*bHasSolverSiblings = true;

			if ( lookupIndex != NULL && (*lookupIndex) < 0 )
				*lookupIndex = s;
		}
	}

	for ( int i = 0; i < _texList.Count(); i++ )
	{
		SimpleTexture *curTex = _texList[i];

		for ( int a = 0; a < curTex->m_hTargetNodes.Count(); a++ )
		{
			HNODE *texID = curTex->m_hTargetNodes[ a ];
			if ( *texID == GetData().iNodeIndex )
			{
				Assert( !activeTex );
				activeTex = curTex;
				break;
			}
		}
	}

	Assert( activeTex );
	return activeTex;
}

void CHLSL_Solver_TextureSample::OnVarInit( const WriteContext_FXC &context )
{
	Assert( context.m_hActive_Solvers );
	Assert( context.m_pActive_Identifiers );
	if ( !m_bIsSamplerObject )
		return;

	SimpleTexture *activeTex = GetActiveTexture( context );

	char samplername[32];
	Q_snprintf( samplername, 32, "_Sampler_%02i", activeTex->iSamplerIndex );
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->SetName( samplername, true );
}
void CHLSL_Solver_TextureSample::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	Assert( context.m_hActive_Solvers );
	Assert( context.m_pActive_Identifiers );
	if ( m_bIsSamplerObject )
		return;

	bool bHasSolverSiblings;
	bool bIsFirstSibling;
	int lookupIndex;
	SimpleTexture *activeTex = GetActiveTexture( context, &bHasSolverSiblings, &bIsFirstSibling, &lookupIndex );

	const bool bDoFunctionOverride = iFunctionOverride != TEXSAMP_AUTO;

	char tmp[MAXTARGC];

	char sharedname[32];
	Q_snprintf( sharedname, 32, "_texLookup_%i", lookupIndex );
	char lookupmode[32];
	Q_snprintf( lookupmode, 32, "%s", activeTex->bCubeTexture ? "texCUBE" : "tex2D" );
	if ( bDoFunctionOverride )
	{
		Q_snprintf( lookupmode, 32, "%s", GetSamplerData_CodeName( iFunctionOverride ) );
	}
	char samplername[32];
	Q_snprintf( samplername, 32, "_Sampler_%02i", activeTex->iSamplerIndex );

	CHLSL_Var *tg = GetTargetVar( 0 );
	CHLSL_Var *src1 = GetSourceVar( 0 );

	char composedlookupstring[MAX_PATH];
	if ( !GetSamplerData_UseDerivative( iFunctionOverride ) )
		Q_snprintf( composedlookupstring, MAX_PATH, "%s( %s, %s )", lookupmode, samplername, src1->GetName() );
	else
		Q_snprintf( composedlookupstring, MAX_PATH, "%s( %s, %s, %s, %s )", lookupmode, samplername,
		src1->GetName(), GetSourceVar(1)->GetName(), GetSourceVar(2)->GetName() );

	// declare a shared lookup
	if ( bHasSolverSiblings && bIsFirstSibling )
	{
		Q_snprintf( tmp, MAXTARGC, "float4 %s = %s;\n",
			sharedname,
			composedlookupstring );
		context.buf_code.PutString( tmp );
	}
	// read from a shared lookup
	if ( bHasSolverSiblings )
	{
		tg->DeclareMe(context);
		Q_snprintf( tmp, MAXTARGC, "%s = %s.%s;\n",
			tg->GetName(),
			sharedname,
			ReadModeToChar[iLookupMode] );
		context.buf_code.PutString( tmp );
	}
	// immediate lookup!
	else
	{
		if ( false )
		{
			Q_snprintf( tmp, MAXTARGC, "%s.%s",
				composedlookupstring,
				ReadModeToChar[iLookupMode] );
			tg->SetName( tmp );
		}
		else
		{
			tg->DeclareMe(context);
			Q_snprintf( tmp, MAXTARGC, "%s = %s.%s;\n",
				tg->GetName(),
				composedlookupstring,
				ReadModeToChar[iLookupMode] );
			context.buf_code.PutString( tmp );
		}
	}
}
void CHLSL_Solver_TextureSample::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleTexture *sampler = new SimpleTexture();
	sampler->bCubeTexture = bCubemap;
	sampler->iTextureMode = iTextureMode;
	sampler->bSRGB = bSrgb;
	sampler->m_hTargetNodes.AddToTail( new HNODE( GetData().iNodeIndex ) );

	AutoCopyStringPtr( szCustomTextureName, &sampler->szTextureName );
	AutoCopyStringPtr( szCustomParamName, &sampler->szParamName );

	if ( sampler->szTextureName != NULL )
		AllocCheck_AllocS( "szTextureName" );

	if ( szFallbackName && Q_strlen( szFallbackName ) )
	{
		char tmp[MAX_PATH];
		Q_StripExtension( szFallbackName, tmp, MAX_PATH );
		sampler->szFallbackName = new char[ Q_strlen( tmp ) + 1 ];
		Q_strcpy( sampler->szFallbackName, tmp );
	}
	List.hList_Textures.AddToTail( sampler );
}

void CHLSL_Solver_TextureSample::Render( Preview2DContext &c )
{
	Assert( pRenderMaterial );

#if DEBUG
	if ( !pRenderMaterial )
		return;
#endif

	if ( m_bIsSamplerObject )
	{
	}
	else
	{
		//CreateSolverMaterial();

		CHLSL_Var *src1 = GetSourceVar( 0 );
		int curindex = src1->GetMapIndex();
		if ( curindex != lastSourceIndex )
		{
			Vector2D a,b;
			GetUVsForPuzzle( curindex, a, b );
			pKV->SetString( "$INPUT_UVS_0", VarArgs( "[%f %f %f %f]", a.x, a.y, b.x, b.y ) );
			pRenderMaterial->Refresh();
			//pKV->Clear();

			//bool bDummy;
			//IMaterialVar *pVar = pRenderMaterial->FindVar( "$INPUT_UVS_0", &bDummy );
			//pVar->SetVecValue( a.x, a.y, b.x, b.y );
			//pRenderMaterial->Refresh();
			//SetMaterialVar_Vec( pRenderMaterial, "$INPUT_UVS_0", a.x, a.y, b.x, b.y );
			
			lastSourceIndex = curindex;
		}
	}

	CNodeView::RenderSingleSolver( c, pRenderMaterial );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_Solver_TextureSample::OnRenderInit()
{
	CreateSolverMaterial();
}

void CHLSL_Solver_TextureSample::DestroySolverMaterial()
{
	if ( !pRenderMaterial )
	{
		Assert( !pKV );
		return;
	}

	Assert( pKV );

	pKV->Clear();
	ForceDeleteMaterial( &pRenderMaterial );
	pKV = NULL;
}

void CHLSL_Solver_TextureSample::CreateSolverMaterial()
{
	DestroySolverMaterial();

	Assert( !pKV );

	pKV = new KeyValues( "NODE_CALC" );

	//pRenderMaterial = materials->CreateMaterial( "__texsampler", pKV );
	//pRenderMaterial->Refresh();

	//return;
	//SetMaterialVar_Int( pRenderMaterial, "$OPERATION", bCubemap ? NPSOP_TEXTURE_LOOKUP_3D : NPSOP_TEXTURE_LOOKUP_2D );
	pKV->SetInt( "$OPERATION", bCubemap ? NPSOP_TEXTURE_LOOKUP_3D : NPSOP_TEXTURE_LOOKUP_2D );

	if ( m_bIsSamplerObject || !GetNumSourceVars() )
		//SetMaterialVar_Int( pRenderMaterial, "$OPERATION", NPSOP_TEXTURE_LOOKUP_FIXED );
		pKV->SetInt( "$OPERATION", NPSOP_TEXTURE_LOOKUP_FIXED );

	//SetMaterialVar_Int( pRenderMaterial, "$TEXTURETYPE", iTextureMode );
	//SetMaterialVar_Int( pRenderMaterial, "$ISCUBEMAP", bCubemap ? 1: 0 );
	//SetMaterialVar_Int( pRenderMaterial, "$SRGB", bSrgb ? 1: 0 );

	pKV->SetInt( "$TEXTURETYPE", iTextureMode );
	pKV->SetInt( "$ISCUBEMAP", bCubemap ? 1: 0 );
	pKV->SetInt( "$SRGB", bSrgb ? 1: 0 );
	//if ( szCustomTextureName && Q_strlen( szCustomTextureName ) )
	//	SetMaterialVar_String( pRenderMaterial, "$CUSTOMTEXTURE", szCustomTextureName );
	//else
	//	SetMaterialVar_String( pRenderMaterial, "$CUSTOMTEXTURE", "" );

	KeyValues *pSub = pKV->FindKey( "$CUSTOMTEXTURE" );

	if ( szCustomTextureName && Q_strlen( szCustomTextureName ) )
		pKV->SetString( "$CUSTOMTEXTURE", szCustomTextureName );
	else if ( pSub != NULL )
		pKV->RemoveSubKey( pSub );
		//pKV->SetString( "$CUSTOMTEXTURE", "" );

	const bool _r_r =	iLookupMode == TEXSAMPLER_LOOKUPMODE_RGBA ||
						iLookupMode == TEXSAMPLER_LOOKUPMODE_RGB ||
						iLookupMode == TEXSAMPLER_LOOKUPMODE_R;

	const bool _g_g =	iLookupMode == TEXSAMPLER_LOOKUPMODE_RGBA ||
						iLookupMode == TEXSAMPLER_LOOKUPMODE_RGB;
	const bool _g_r =	iLookupMode == TEXSAMPLER_LOOKUPMODE_G;

	const bool _b_b =	iLookupMode == TEXSAMPLER_LOOKUPMODE_RGBA ||
						iLookupMode == TEXSAMPLER_LOOKUPMODE_RGB;
	const bool _b_r =	iLookupMode == TEXSAMPLER_LOOKUPMODE_B;

	const bool _a_a =	iLookupMode == TEXSAMPLER_LOOKUPMODE_RGBA;
	const bool _a_r =	iLookupMode == TEXSAMPLER_LOOKUPMODE_A;

	//SetMaterialVar_Vec( pRenderMaterial, "$INPUT_UVS_1", _r_r, _g_r, _b_r, _a_r );
	//SetMaterialVar_Vec( pRenderMaterial, "$INPUT_UVS_2", 0, _g_g, 0, 0 );
	//SetMaterialVar_Vec( pRenderMaterial, "$INPUT_UVS_3", 0, 0, _b_b, 0 );
	//SetMaterialVar_Vec( pRenderMaterial, "$INPUT_UVS_4", 0, 0, 0, _a_a );

	pKV->SetString( "$INPUT_UVS_1", VarArgs( "[%d %d %d %d]", _r_r, _g_r, _b_r, _a_r ) );
	pKV->SetString( "$INPUT_UVS_2", VarArgs( "[%d %d %d %d]", 0, _g_g, 0, 0 ) );
	pKV->SetString( "$INPUT_UVS_3", VarArgs( "[%d %d %d %d]", 0, 0, _b_b, 0 ) );
	pKV->SetString( "$INPUT_UVS_4", VarArgs( "[%d %d %d %d]", 0, 0, 0, _a_a ) );

	pRenderMaterial = materials->CreateMaterial( "__texsampler", pKV );

	pRenderMaterial->Refresh();
	pRenderMaterial->RecomputeStateSnapshots();

	//pKV->Clear();
}
