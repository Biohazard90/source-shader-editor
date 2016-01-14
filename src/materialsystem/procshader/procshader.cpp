
#include "shaderincludes.h"
#include "convar.h"

#include "fallback_vs20.inc"
#include "fallback_ps20.inc"

#include "../procshaderinterface.h"
#include "cpp_shader_constant_register_map.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef SHADER_EDITOR_DLL_2006
class CProceduralContext
{
};
#else
class CProceduralContext : public CBasePerMaterialContextData
{
public:
	CProceduralContext()
	{
		flLightmapScaleFactor = 1.0f;
	};

	virtual ~CProceduralContext()
	{
	};

	float flLightmapScaleFactor;
};
#endif

void UpdateConstantByIdentifier( CBaseVSShader *pShader, IShaderDynamicAPI* pShaderAPI, IMaterialVar **params, SimpleEnvConstant *pConst, CProceduralContext *pContext,
							bool bPS, int iFirstMutable, int iFirstStatic )
{
	float data[4][4] = {
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};

	int _register = RemapEnvironmentConstant( bPS, pConst->iHLSLRegister );

	switch ( pConst->iEnvC_ID )
	{
	default:
		Assert(0);
	case HLSLENV_TIME:
		data[0][ 0 ] = pShaderAPI->CurrentTime();
		break;
	case HLSLENV_VIEW_ORIGIN:
	case HLSLENV_VIEW_FWD:
	case HLSLENV_VIEW_RIGHT:
	case HLSLENV_VIEW_UP:
	case HLSLENV_VIEW_WORLDDEPTH:
		Q_memcpy( data, gProcShaderCTRL->AccessEnvConstant( pConst->iEnvC_ID ), sizeof(float)*4 );
		break;
	case HLSLENV_PIXEL_SIZE:
		{
			int bx, by;
			pShaderAPI->GetBackBufferDimensions( bx, by );
			float scale = max( 1.0f, pConst->flSmartDefaultValues[0] );
			data[0][ 0 ] = ( 1.0f / bx ) * scale;
			data[0][ 1 ] = ( 1.0f / by ) * scale;
		}
		break;
	case HLSLENV_FOG_PARAMS:
		Assert( bPS );
		pShaderAPI->SetPixelShaderFogParams( _register );
		return;
	case HLSLENV_STUDIO_LIGHTING_VS:
		//if ( pShader->UsingFlashlight( params ) )
		//	return;
#ifndef SHADER_EDITOR_DLL_SWARM
		pShaderAPI->SetVertexShaderStateAmbientLightCube();
#else
		pShader->PI_SetVertexShaderAmbientLightCube();
		pShader->PI_SetVertexShaderLocalLighting();
#endif
		return;
	case HLSLENV_STUDIO_LIGHTING_PS:
		if ( pShader->UsingFlashlight( params ) )
			return;
#ifndef SHADER_EDITOR_DLL_SWARM
		pShaderAPI->SetPixelShaderStateAmbientLightCube( SSEREG_AMBIENT_CUBE );
		pShaderAPI->CommitPixelShaderLighting( SSEREG_LIGHT_INFO_ARRAY );
#else
		pShader->PI_SetPixelShaderAmbientLightCube( SSEREG_AMBIENT_CUBE );
		pShader->PI_SetPixelShaderLocalLighting( SSEREG_LIGHT_INFO_ARRAY );
#endif
		return;
	case HLSLENV_FLASHLIGHT_VPMATRIX:
		{
			VMatrix worldToTexture;
			pShaderAPI->GetFlashlightState( worldToTexture );
			Q_memcpy( data, worldToTexture.Base(), sizeof(float)*16 );
			if ( bPS )
				_register = SSEREG_FLASHLIGHT_TO_WORLD_TEXTURE;
		}
		break;
	case HLSLENV_FLASHLIGHT_DATA:
		{
			if ( !pShader->UsingFlashlight( params ) )
				return;
			Assert( bPS );
			VMatrix dummy;
			const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState( dummy );
#ifndef SHADER_EDITOR_DLL_2006
			float tweaks[4];
			tweaks[0] = flashlightState.m_flShadowFilterSize / flashlightState.m_flShadowMapResolution;
			tweaks[1] = ShadowAttenFromState( flashlightState );
			pShader->HashShadow2DJitter( flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3] );
			pShaderAPI->SetPixelShaderConstant( SSEREG_LIGHT_INFO_ARRAY+1, tweaks, 1 ); // c07
#endif
			float vScreenScale[4] = {1280.0f / 32.0f, 768.0f / 32.0f, 0, 0};
			int nWidth, nHeight;
			pShaderAPI->GetBackBufferDimensions( nWidth, nHeight );
			vScreenScale[0] = (float) nWidth  / 32.0f;
			vScreenScale[1] = (float) nHeight / 32.0f;
			pShaderAPI->SetPixelShaderConstant( SSEREG_LIGHT_INFO_ARRAY + 5, vScreenScale, 1 ); // c11
#ifdef SHADER_EDITOR_DLL_2006
			SetFlashLightColorFromState( flashlightState, pShaderAPI, SSEREG_LIGHT_INFO_ARRAY + 4 ); // c10
#else
			SetFlashLightColorFromState( flashlightState, pShaderAPI, SSEREG_LIGHT_INFO_ARRAY + 4, false ); // c10
#endif

			float atten_pos[8];
			atten_pos[0] = flashlightState.m_fConstantAtten;			// c08
			atten_pos[1] = flashlightState.m_fLinearAtten;
			atten_pos[2] = flashlightState.m_fQuadraticAtten;
			atten_pos[3] = flashlightState.m_FarZ;
			atten_pos[4] = flashlightState.m_vecLightOrigin[0];			// c09
			atten_pos[5] = flashlightState.m_vecLightOrigin[1];
			atten_pos[6] = flashlightState.m_vecLightOrigin[2];
			atten_pos[7] = 1.0f;
			pShaderAPI->SetPixelShaderConstant( SSEREG_LIGHT_INFO_ARRAY+2, atten_pos, 2 );
			// PSREG_LIGHT_INFO_ARRAY + 3 !!
		}
		return;
	case HLSLENV_SMART_CALLBACK:
		{
			int index = pConst->iFastLookup;
			if ( index < 0 )
				return;
			Assert( pConst->szSmartHelper );
			pFnClCallback( func ) = gProcShaderCTRL->GetCallback( index )->func;
			func( &data[0][0] );
		}
		break;
	case HLSLENV_SMART_VMT_MUTABLE:
		{
			if ( iFirstMutable < 0 )
				break;
			Assert( pConst->iFastLookup >= 0 && pConst->iFastLookup < AMT_VMT_MUTABLE );
			params[ iFirstMutable + pConst->iFastLookup ]->GetVecValue( &data[0][0], 4 );
		}
		break;
	case HLSLENV_SMART_VMT_STATIC:
		{
			if ( iFirstStatic < 0 )
				break;
			Assert( pConst->iFastLookup >= 0 && pConst->iFastLookup < AMT_VMT_STATIC );
			params[ iFirstStatic + pConst->iFastLookup ]->GetVecValue( &data[0][0], 4 );
		}
		break;
	case HLSLENV_SMART_RANDOM_FLOAT:
		{
			Assert( pConst->iSmartNumComps >= 0 && pConst->iSmartNumComps <= 3 );
			for ( int i = 0; i <= pConst->iSmartNumComps; i++ )
				data[0][i] = RandomFloat( pConst->flSmartDefaultValues[0], pConst->flSmartDefaultValues[1] );
		}
		break;
	case HLSLENV_CUSTOM_MATRIX:
		{
			VMatrix matTmp, matTmpTranspose;
			switch ( pConst->iSmartNumComps )
			{
			case CMATRIX_VIEW:
				pShaderAPI->GetMatrix( MATERIAL_VIEW, matTmp.Base() );
				break;
			case CMATRIX_PROJ:
				pShaderAPI->GetMatrix( MATERIAL_PROJECTION, matTmp.Base() );
				break;
			case CMATRIX_VIEWPROJ:
				{
					VMatrix matV, matP;
					pShaderAPI->GetMatrix( MATERIAL_VIEW, matV.Base() );
					pShaderAPI->GetMatrix( MATERIAL_PROJECTION, matP.Base() );
					MatrixMultiply( matV, matP, matTmp );
				}
				break;
			case CMATRIX_VIEW_INV:
				{
					VMatrix matPre;
					pShaderAPI->GetMatrix( MATERIAL_VIEW, matPre.Base() );
					MatrixInverseGeneral( matPre, matTmp );
				}
				break;
			case CMATRIX_PROJ_INV:
				{
					VMatrix matPre;
					pShaderAPI->GetMatrix( MATERIAL_PROJECTION, matPre.Base() );
					MatrixInverseGeneral( matPre, matTmp );
				}
				break;
			case CMATRIX_VIEWPROJ_INV:
				{
					VMatrix matV, matP, matPre;
					pShaderAPI->GetMatrix( MATERIAL_VIEW, matV.Base() );
					pShaderAPI->GetMatrix( MATERIAL_PROJECTION, matP.Base() );
					MatrixMultiply( matV, matP, matPre );
					MatrixInverseGeneral( matPre, matTmp );
				}
				break;
			}
			MatrixTranspose( matTmp, matTmpTranspose );
			Q_memcpy( &data[0][0], matTmpTranspose.Base(), sizeof(float) * 16 );
		}
		break;
	case HLSLENV_LIGHTMAP_RGB:
#ifdef SHADER_EDITOR_DLL_SWARM
		if ( pContext )
			data[0][0] = pContext->flLightmapScaleFactor;
		else
			data[0][0] = 1.0f;
#elif SHADER_EDITOR_DLL_2006
#else
			data[0][0] = pShaderAPI->GetLightMapScaleFactor();
#endif
		break;
	}

	Assert( pConst->iConstSize >= 1 && pConst->iConstSize <= 4 );
	Assert( _register >= 0 );

	if ( bPS )
		pShaderAPI->SetPixelShaderConstant( _register, &data[0][0], pConst->iConstSize );
	else
		pShaderAPI->SetVertexShaderConstant( _register, &data[0][0], pConst->iConstSize );
}

int CalcShaderIndex( CBaseVSShader *pShader, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression,
	IMaterialVar **params,
	const CUtlVector< SimpleCombo* > &hList, bool bStatic, int numDynamic, bool bPreview )
{
#ifndef SHADER_EDITOR_DLL_2006
	LightState_t lState = { 0, false, false };
	if ( pShaderAPI && !pShader->UsingFlashlight( params ) )
		pShaderAPI->GetDX9LightState( &lState );
#endif
	int localIndex = 0;
	int curMultiplier = bStatic ? numDynamic : 1;

	int iLeft = hList.Count();
	SimpleCombo *c = NULL;

	for ( SimpleCombo *const*cl = hList.Base();
		iLeft > 0;
		iLeft--, cl++ )
	{
		c = *cl;

		if ( bPreview && !c->bInPreviewMode )
			continue;

		if ( c->bStatic != bStatic )
			continue;

		int localCombo = 0;
		switch ( c->iComboType )
		{
		default:
#ifndef SHADER_EDITOR_DLL_2006
			Assert( 0 );
#endif
			break;
		case HLSLCOMBO_CUSTOM:
			break;
#ifndef SHADER_EDITOR_DLL_2006
		case HLSLCOMBO_LIGHT_STATIC:
			Assert( pShaderAPI );
			localCombo = lState.m_bStaticLight ? 1 : 0;
			break;
		case HLSLCOMBO_LIGHT_DYNAMIC:
			Assert( pShaderAPI );
			localCombo = lState.HasDynamicLight() ? 1 : 0;
			break;
		case HLSLCOMBO_NUM_LIGHTS:
			Assert( pShaderAPI );
			localCombo = lState.m_nNumLights;
			break;
#endif
		case HLSLCOMBO_SKINNING:
			Assert( pShaderAPI );
			localCombo = ( pShaderAPI->GetCurrentNumBones() > 0 ) ? 1 : 0;
			break;
#ifndef SHADER_EDITOR_DLL_2006
		case HLSLCOMBO_MORPHING:
			Assert( pShaderAPI );
			localCombo = pShaderAPI->IsHWMorphingEnabled() ? 1 : 0;
			break;
		case HLSLCOMBO_VERTEXCOMPRESSION:
			localCombo = vertexCompression;
			break;
		case HLSLCOMBO_PIXELFOG:
			Assert( pShaderAPI );
			localCombo = pShaderAPI->GetPixelFogCombo() ? 1 : 0;
			break;
#endif
		case HLSLCOMBO_WATERFOG_TOALPHA:
			{
				Assert( pShaderAPI );
				MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
				localCombo = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) ? 1 : 0;
			}
			break;
		case HLSLCOMBO_FLASHLIGHT_ENABLED:
			localCombo = pShader->UsingFlashlight( params ) ? 1 : 0;
			break;
#ifndef SHADER_EDITOR_DLL_2006
		case HLSLCOMBO_FLASHLIGHT_FILTER_MODE:
			if ( pShader->UsingFlashlight( params ) )
				localCombo = g_pHardwareConfig->GetShadowFilterMode();
			break;
		case HLSLCOMBO_FLASHLIGHT_DO_SHADOWS:
			if ( pShader->UsingFlashlight( params ) )
			{
#ifdef SHADER_EDITOR_DLL_SWARM
				bool bShadows, bUber;
				pShaderAPI->GetFlashlightShaderInfo( &bShadows, &bUber );
				localCombo = bShadows ? 1 : 0;
#else
				Assert( pShaderAPI );
				VMatrix dummy;
				const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState( dummy );
				localCombo = flashlightState.m_bEnableShadows ? 1 : 0;
#endif
			}
			break;
#endif
		}

		localIndex += curMultiplier * localCombo;
		curMultiplier *= c->GetAmt();
	}

	return localIndex;
}

void DrawFallback( CBaseVSShader *pShader, IMaterialVar** params, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression )
{
	SHADOW_STATE
	{
		pShaderShadow->SetDefaultState();

		pShaderShadow->EnableDepthWrites( true );

		int fmt = VERTEX_POSITION;
#ifndef SHADER_EDITOR_DLL_2006
		fmt |= VERTEX_FORMAT_COMPRESSED;
#endif
		VERTEXSHADERVERTEXFORMAT( fmt, 1, 0, 0 );

		pShaderShadow->SetVertexShader( "fallback_vs20", 0 );
		pShaderShadow->SetPixelShader( "fallback_ps20", 0 );
	}

	DYNAMIC_STATE
	{
#ifdef SHADER_EDITOR_DLL_SWARM
		pShader->PI_BeginCommandBuffer();
#endif

		pShaderAPI->SetDefaultState();

#ifdef SHADER_EDITOR_DLL_2006
		bool bIsCompressed = false;
#else
		bool bIsCompressed = vertexCompression == VERTEX_COMPRESSION_ON;
#endif

		pShaderAPI->SetVertexShaderIndex( bIsCompressed ? 0 : 0 );
		pShaderAPI->SetPixelShaderIndex();

#ifdef SHADER_EDITOR_DLL_SWARM
		pShader->PI_EndCommandBuffer();
#endif
	}

#ifdef SHADER_EDITOR_DLL_2006
	pShader->Draw();
#else
	pShader->Draw( !pShader->UsingFlashlight( params ) );
#endif
}

DEFINE_FALLBACK_SHADER( LIGHTMAPPEDGENERIC_EDITOR_SHADER, EDITOR_SHADER );

BEGIN_VS_SHADER( EDITOR_SHADER, "" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( TEXTURE_0, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_1, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_2, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_3, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_4, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_5, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_6, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_7, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_8, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_9, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_10, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_11, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_12, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_13, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_14, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_15, SHADER_PARAM_TYPE_TEXTURE, "", "" )

		SHADER_PARAM( TEXTURE_VS_0, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_VS_1, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_VS_2, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( TEXTURE_VS_3, SHADER_PARAM_TYPE_TEXTURE, "", "" )

		SHADER_PARAM( MUTABLE_01, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( MUTABLE_02, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( MUTABLE_03, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( MUTABLE_04, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( MUTABLE_05, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( MUTABLE_06, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( MUTABLE_07, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( MUTABLE_08, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( MUTABLE_09, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( MUTABLE_10, SHADER_PARAM_TYPE_VEC4, "", "" )

		SHADER_PARAM( VPSTATIC_00, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_01, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_02, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_03, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_04, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_05, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_06, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_07, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_08, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_09, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_10, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_11, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_12, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_13, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_14, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( VPSTATIC_15, SHADER_PARAM_TYPE_VEC4, "", "" )

		SHADER_PARAM( VPSTATIC_00_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_01_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_02_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_03_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_04_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_05_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_06_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_07_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_08_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_09_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_10_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_11_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_12_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_13_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_14_NAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( VPSTATIC_15_NAME, SHADER_PARAM_TYPE_STRING, "", "" )

		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "" )
		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "", "" )

		SHADER_PARAM( SHADERNAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( ISCUSTOMSHADERREADY, SHADER_PARAM_TYPE_BOOL, "0", "" )
		SHADER_PARAM( CUSTOMSHADERINDEX, SHADER_PARAM_TYPE_INTEGER, "", "" )

		SHADER_PARAM( SHADERLIGHTMAPMODE, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( __PPE_INLINE, SHADER_PARAM_TYPE_INTEGER, "0", "" )
	END_SHADER_PARAMS

#ifndef SHADER_EDITOR_DLL_SWARM
	const SoftwareVertexShader_t GetSoftwareVertexShader() const { return NULL; }
#endif
	bool NeedsPowerOfTwoFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame = true ) const
	{
#ifdef SHADER_EDITOR_DLL_2006
		return false;
#else
		return IS_FLAG2_SET( MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE ) ||
			NeedsFullFrameBufferTexture( params, bCheckSpecificToThisFrame );
#endif
	}
	bool NeedsFullFrameBufferTexture( IMaterialVar** params, bool bCheckSpecificToThisFrame = true ) const
	{
#ifdef SHADER_EDITOR_DLL_2006
		return false;
#else
		return IS_FLAG2_SET( MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE );
#endif
	}
	//bool IsTranslucent( IMaterialVar **params ) const
	//{
	//	return IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT );
	//}

	bool IsCustomShader( IMaterialVar **params )
	{
		return params[ISCUSTOMSHADERREADY]->IsDefined() && params[ISCUSTOMSHADERREADY]->GetIntValue() > 0;
	}

	bool IsInlineMaterial( IMaterialVar **params )
	{
		return params[PPEINLINE_PARAM_KEY]->IsDefined() && params[PPEINLINE_PARAM_KEY]->GetIntValue() > 0;
	}

	BasicShaderCfg_t *GetCustomShaderData( IMaterialVar **params )
	{
		if ( !params[ISCUSTOMSHADERREADY] )
			return NULL;
		return gProcShaderCTRL->GetPreloadShader_Internal( params[CUSTOMSHADERINDEX]->GetIntValue() );
	}

	int GetParamIdxFromMode( int texmode )
	{
		switch ( texmode )
		{
		case HLSLTEX_BUMPMAP:
			return BUMPMAP;
		case HLSLTEX_ENVMAP:
			return ENVMAP;
		case HLSLTEX_BASETEXTURE:
			return BASETEXTURE;
		}
		return -1;
	}
	void InitFlags2( BasicShaderCfg_t *cfg, IMaterialVar **params )
	{
		params[FLAGS]->SetIntValue( 0 );
		params[FLAGS2]->SetIntValue( 0 );
		params[FLAGS_DEFINED]->SetIntValue( 0 );
		params[FLAGS_DEFINED2]->SetIntValue( 0 );

		if ( cfg->bVertexLighting )
		{
			CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
			SET_FLAGS( MATERIAL_VAR_MODEL );

			CLEAR_FLAGS2( MATERIAL_VAR2_LIGHTING_UNLIT );
			SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
			SET_FLAGS2( MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL );
			SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
		}

#ifndef SHADER_EDITOR_DLL_2006
		if ( cfg->bRefractionSupport )
			SET_FLAGS2( MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE );
#endif

		if ( cfg->iVFMT_flags & VERTEX_TANGENT_S || cfg->iVFMT_flags & VERTEX_TANGENT_T || cfg->iVFMT_flags & VERTEX_NORMAL )
			SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );

		if ( ABLEND_IS_TRANSLUCENT( cfg->iAlphablendmode ) )
			SET_FLAGS( MATERIAL_VAR_TRANSLUCENT );

		if ( cfg->iVFMT_numTexcoords > 1 )
			SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );
		if ( cfg->iVFMT_numTexcoords > 2 )
			SET_FLAGS2( MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP );

#ifndef SHADER_EDITOR_DLL_2006
		for ( int i = 0; i < cfg->pVS_Identifiers->hList_Combos.Count(); i++ )
		{
			SimpleCombo *c = cfg->pVS_Identifiers->hList_Combos[i];
			switch ( c->iComboType )
			{
			case HLSLCOMBO_MORPHING:
					if ( g_pHardwareConfig->HasFastVertexTextures() )
						SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );
				break;
			case HLSLCOMBO_SKINNING:
					SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
				break;
			}
		}
		for ( int i = 0; i < cfg->pPS_Identifiers->hList_Combos.Count(); i++ )
		{
			SimpleCombo *c = cfg->pPS_Identifiers->hList_Combos[i];
			switch ( c->iComboType )
			{
			case HLSLCOMBO_FLASHLIGHT_ENABLED:
					SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_FLASHLIGHT );
				break;
			}
		}
#endif

		for ( int i = 0; i < cfg->pPS_Identifiers->hList_Textures.Count(); i++ )
		{
			SimpleTexture *tex = cfg->pPS_Identifiers->hList_Textures[i];
			switch ( tex->iTextureMode )
			{
			case HLSLTEX_ENVMAP:
					SET_FLAGS2( MATERIAL_VAR2_USES_ENV_CUBEMAP );
				break;
			}
		}
	}

	void InitSmartConstants_Names( IMaterialVar **params )
	{
		for ( int i = 0; i < AMT_VMT_STATIC; i++ )
		{
			params[ VPSTATIC_00_NAME + i ]->SetStringValue( "" );
		}
	}
	bool ParseVec4FromParam( const char *pszString, Vector4D &val )
	{
#define __IS_NUM( x ) ( ( x >= ((int)'0') && x <= ((int)'9') ) || x == '.' || x == '-' )

		val.Init();

		const char *pRead = pszString;
		if ( pRead && *pRead && Q_strlen( pRead ) )
		{
			int curSlot = 0;
			bool bPass = false;
			while ( pRead && *pRead )
			{
				const bool bNum = __IS_NUM( *pRead );
				if ( !bNum ||
					curSlot >= 4 ||
					bPass )
				{
					if ( bPass && !bNum )
						bPass = false;
					pRead++;
					continue;
				}
				val[ curSlot ] = atof( pRead );
				curSlot++;
				bPass = true;
			}

			return true;
		}

		return false;
	}
	void InitSmartConstants_Static( CUtlVector < SimpleEnvConstant* > &hList, IMaterialVar **params, KeyValues *data, bool bPreloaded )
	{
		for ( int i = 0; i < hList.Count(); i++ )
		{
			SimpleEnvConstant *c = hList[i];
			switch (c->iEnvC_ID)
			{
			case HLSLENV_SMART_CALLBACK:
				Assert( c->szSmartHelper );
				{
					int index = gProcShaderCTRL->FindCallback( c->szSmartHelper );
					c->iFastLookup = index;
				}
				break;
			case HLSLENV_SMART_VMT_STATIC:
				Assert( c->szSmartHelper );
				{
					int iNewIndex = 0;
					int iFoundIndex = -1;

					for ( int i = 0; i < AMT_VMT_STATIC && iFoundIndex < 0; i++ )
					{
						if ( !params[ VPSTATIC_00_NAME + i ]->GetStringValue() )
							continue;
						if ( !Q_stricmp( params[ VPSTATIC_00_NAME + i ]->GetStringValue(), c->szSmartHelper ) )
							iFoundIndex = i;
					}

					if ( iFoundIndex < 0 )
					{
						for ( int i = 0; i < AMT_VMT_STATIC; i++ )
						{
							if ( !params[ VPSTATIC_00_NAME + i ]->GetStringValue() ||
								!*params[ VPSTATIC_00_NAME + i ]->GetStringValue() )
								break;
							iNewIndex++;
						}
					}

					int iTargetIndex = ( iFoundIndex >= 0 ) ? iFoundIndex : iNewIndex;
					Assert( iTargetIndex >= 0 && iTargetIndex < AMT_VMT_STATIC );

					Assert( ( iFoundIndex >= 0 ) ||
						!params[ VPSTATIC_00_NAME + iTargetIndex ]->GetStringValue() ||
						!*params[ VPSTATIC_00_NAME + iTargetIndex ]->GetStringValue() );

					params[ VPSTATIC_00_NAME + iTargetIndex ]->SetStringValue( c->szSmartHelper );
					params[ VPSTATIC_00 + iTargetIndex ]->SetVecValue( c->flSmartDefaultValues, 4 );

					const char *name = c->szSmartHelper;
					char tmp[MAX_PATH];
					Q_snprintf( tmp, MAX_PATH, "$%s", name );
					const char *input = data->GetString( tmp );

					Vector4D val;
					if ( ParseVec4FromParam( input, val ) )
						params[ VPSTATIC_00 + iTargetIndex ]->SetVecValue( val.Base(), 4 );

					c->iFastLookup = iTargetIndex;
				}
				break;
			case HLSLENV_SMART_VMT_MUTABLE:
				Assert( c->szSmartHelper );
				Assert( Q_strlen(c->szSmartHelper) > 2 );

				const char *NumStart = c->szSmartHelper;
				NumStart += Q_strlen( NumStart ) - 2;
				c->iFastLookup = atoi( NumStart ) - 1;

				Assert( c->iFastLookup >= 0 && c->iFastLookup < AMT_VMT_MUTABLE );

				params[ MUTABLE_01 + c->iFastLookup ]->SetVecValue( c->flSmartDefaultValues, 4 );

				char paramName[MAX_PATH];
				Q_snprintf( paramName, sizeof( paramName ), "$%s", c->szSmartHelper );
				const char *input = data->GetString( paramName );

				Vector4D val;
				if ( ParseVec4FromParam( input, val ) )
					params[ MUTABLE_01 + c->iFastLookup ]->SetVecValue( val.Base(), 4 );

				break;
			}
		}
	}

	const char *GetFlashlightTexture()
	{
#if defined(SHADER_EDITOR_DLL_SWARM) || defined(SHADER_EDITOR_DLL_2006)
		return "effects/flashlight001";
#else
		return g_pHardwareConfig->SupportsBorderColor() ? "effects/flashlight_border" : "effects/flashlight001";
#endif
	}

	void ShaderInit_Textures( CUtlVector < SimpleTexture* > &hList, IMaterialVar **params, KeyValues *data, int startRegister )
	{
		const bool bCustomShader = IsCustomShader(params);
		for ( int i = 0; i < hList.Count(); i++ )
		{
			SimpleTexture *_tex = hList[i];
			if ( !::IsTextypeUsingCustomTexture( _tex->iTextureMode ) )
				continue;

			const bool bLockedTexture = _tex->iTextureMode == HLSLTEX_FLASHLIGHT_COOKIE;
			if ( !_tex->szTextureName && !bCustomShader && !bLockedTexture )
				continue;

			int idx = startRegister + _tex->iSamplerIndex;

			if ( _tex->iTextureMode == HLSLTEX_FLASHLIGHT_COOKIE )
				params[idx]->SetStringValue( GetFlashlightTexture() );
			else if ( !bCustomShader )
				params[idx]->SetStringValue( _tex->szTextureName );
			else
			{
				int newIdx = GetParamIdxFromMode( _tex->iTextureMode );
				char tmp[MAX_PATH];
				if ( newIdx >= 0 )
				{
					idx = newIdx;
					if ( idx == BASETEXTURE )
						Q_strcpy( tmp, "$BASETEXTURE" );
					else if ( idx == BUMPMAP )
						Q_strcpy( tmp, "$BUMPMAP" );
					else if ( idx == ENVMAP )
						Q_strcpy( tmp, "$ENVMAP" );
					else
						Assert( 0 );
				}
				else
					Q_snprintf( tmp, MAX_PATH, "$%s", _tex->szParamName );

				const char *VmtTexture = data->GetString( tmp );
				if ( VmtTexture && Q_strlen( VmtTexture ) )
					params[idx]->SetStringValue( VmtTexture );
			}

			if ( _tex->bCubeTexture )
				LoadCubeMap( idx );
			else
				LoadTexture( idx );
		}
	}

	SHADER_INIT
	{
		for ( int i = 0; i < AMT_VMT_MUTABLE; i++ )
			params[ MUTABLE_01 + i ]->SetVecValue( 0, 0, 0, 0 );

		BasicShaderCfg_t *cfg = NULL;
		if ( params[SHADERNAME]->IsDefined() && Q_strlen(params[SHADERNAME]->GetStringValue() ) )
		{
			const char *shadername = params[SHADERNAME]->GetStringValue();
			int idx = 0;
			cfg = gProcShaderCTRL->GetPreloadShader_Internal( shadername, &idx );
			params[ISCUSTOMSHADERREADY]->SetIntValue( cfg ? 1 : 0 );
			params[CUSTOMSHADERINDEX]->SetIntValue( idx );
		}
		else
		{
			cfg = gProcShaderCTRL->AccessVolatileData();
		}

		if ( !cfg ) // fallback mode
		{
#ifndef SHADER_EDITOR_DLL_2006
			SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
#endif
			return;
		}

		InitFlags2( cfg, params );

		const bool bCustomShader = IsCustomShader(params);
		const bool bInlineMaterial = bCustomShader && IsInlineMaterial(params);
		KeyValues *pKV = NULL; //new KeyValues("");

		if ( bCustomShader )
		{
			if ( bInlineMaterial )
			{
				if ( !gPPEHelper )
				{
					Warning("shader error - PPEHelper unavailable for material: %s\n", pMaterialName);
					return;
				}
				else
				{
					pKV = gPPEHelper->GetInlineMaterial( pMaterialName );

					if ( pKV )
						pKV = pKV->MakeCopy();
				}
			}
			else
			{
				char localpath[MAX_PATH];
				char fullpath[MAX_PATH];
				Q_snprintf( localpath, MAX_PATH, "materials\\%s.vmt", pMaterialName );
				g_pFullFileSystem->RelativePathToFullPath( localpath, NULL, fullpath, MAX_PATH );

				pKV = new KeyValues("");
				if ( !pKV->LoadFromFile( g_pFullFileSystem, fullpath ) && !pKV->LoadFromFile( g_pFullFileSystem, localpath ) )
				{
					pKV->deleteThis();
					Warning("shader error - cannot load material: %s\n", pMaterialName);
					return;
				}
			}
		}

		if ( !pKV )
		{
			if ( bInlineMaterial )
			{
				Warning("shader error - PPEHelper no KeyValues for material: %s\n", pMaterialName);
				return;
			}

			pKV = new KeyValues("");
		}
		else
			UnpackMaterial( pKV );

		Assert( cfg->pPS_Identifiers );
		ShaderInit_Textures( cfg->pPS_Identifiers->hList_Textures, params, pKV, TEXTURE_0 );
		ShaderInit_Textures( cfg->pVS_Identifiers->hList_Textures, params, pKV, TEXTURE_VS_0 );

		InitSmartConstants_Names( params );
		InitSmartConstants_Static( cfg->pPS_Identifiers->hList_EConstants, params, pKV, bCustomShader );
		InitSmartConstants_Static( cfg->pVS_Identifiers->hList_EConstants, params, pKV, bCustomShader );

		pKV->deleteThis();

		//if ( bInlineMaterial )
		//	gPPEHelper->DestroyKeyValues( pKV );
		//else
		//	pKV->deleteThis();
	}

	void ShaderInitParams_Textures( CUtlVector < SimpleTexture* > &hList, IMaterialVar **params, int startRegister, int numregister )
	{
		for ( int i = 0; i < numregister; i++ )
			params[startRegister+i]->SetUndefined();
		for ( int i = 0; i < hList.Count(); i++ )
		{
			SimpleTexture *_tex = hList[i];
			if ( !::IsTextypeUsingCustomTexture( _tex->iTextureMode ) )
			{
#ifdef SHADER_EDITOR_DLL_SWARM
				if ( _tex->iTextureMode == HLSLTEX_FLASHLIGHT_COOKIE )
				{
					params[FLASHLIGHTTEXTURE]->SetStringValue( GetFlashlightTexture() );
					params[FLASHLIGHTTEXTUREFRAME]->SetIntValue( 0 );
				}
#endif
				continue;
			}
			if ( !_tex->szFallbackName )
				continue;

			int idx = startRegister + _tex->iSamplerIndex;
			int newIdx = GetParamIdxFromMode( _tex->iTextureMode );
			if ( newIdx >= 0 )
				idx = newIdx;
			if ( !params[idx]->IsDefined() || ( params[idx]->GetStringValue() && !*params[idx]->GetStringValue() ) )
				params[idx]->SetStringValue( _tex->szFallbackName );
		}
	}

	SHADER_INIT_PARAMS()
	{
		if ( params[SHADERLIGHTMAPMODE]->IsDefined() && params[SHADERLIGHTMAPMODE]->GetIntValue() > 0 )
		{
			SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );
			if ( params[SHADERLIGHTMAPMODE]->GetIntValue() > 1 )
				SET_FLAGS2( MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP );
		}

		if ( !params[__PPE_INLINE]->IsDefined() )
			params[__PPE_INLINE]->SetIntValue( 0 );

		BasicShaderCfg_t *cfg = NULL;
		if ( params[SHADERNAME]->IsDefined() && Q_strlen(params[SHADERNAME]->GetStringValue() ) )
		{
			const char *shadername = params[SHADERNAME]->GetStringValue();
			cfg = gProcShaderCTRL->GetPreloadShader_Internal( shadername );
		}

		if ( !cfg )
			return;

		if ( cfg->bVertexLighting )
			params[BUMPMAP]->SetStringValue( "" );

#ifndef SHADER_EDITOR_DLL_2006
		CLEAR_FLAGS2( MATERIAL_VAR2_SUPPORTS_FLASHLIGHT );
#endif

		InitFlags2( cfg, params );

		ShaderInitParams_Textures( cfg->pPS_Identifiers->hList_Textures, params, TEXTURE_0, AMT_PS_SAMPLERS );
		ShaderInitParams_Textures( cfg->pVS_Identifiers->hList_Textures, params, TEXTURE_VS_0, AMT_VS_SAMPLERS );
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_DRAW
	{
//#ifdef SHADER_EDITOR_DLL_SWARM
//		Msg( "using flashlight: %i // singlepass: %i at %f\n", UsingFlashlight( params ) ? 1 : 0,
//			pShaderAPI ? pShaderAPI->SinglePassFlashlightModeEnabled() : -1,
//			pShaderAPI ? pShaderAPI->CurrentTime() : -1 );
//#endif

#ifdef SHADER_EDITOR_DLL_2006
		int vertexCompression = 0;
#endif

		BasicShaderCfg_t *cfg = NULL;

		const bool bCustomShader = IsCustomShader( params );
		if ( bCustomShader )
			cfg = GetCustomShaderData( params );
		else
			cfg = gProcShaderCTRL->AccessVolatileData();

		if ( !cfg )
			return DrawFallback( this, params, pShaderShadow, pShaderAPI, vertexCompression );

#ifdef SHADER_EDITOR_DLL_2006
		CProceduralContext *pContext = NULL;
#else
		CProceduralContext *pContext = reinterpret_cast<CProceduralContext*>( *pContextDataPtr );

		if ( !pContext )
		{
			pContext = new CProceduralContext();
			*pContextDataPtr = pContext;
		}
#endif

		Assert( cfg->pPS_Identifiers );
		Assert( cfg->pVS_Identifiers );

		const CUtlVector< SimpleTexture* > &hTextures_PS = cfg->pPS_Identifiers->hList_Textures;
		const CUtlVector< SimpleTexture* > &hTextures_VS = cfg->pVS_Identifiers->hList_Textures;
		const CUtlVector< SimpleEnvConstant* > &hConst_PS = cfg->pPS_Identifiers->hList_EConstants;
		const CUtlVector< SimpleEnvConstant* > &hConst_VS = cfg->pVS_Identifiers->hList_EConstants;
		const CUtlVector< SimpleCombo* > &hCombo_PS = cfg->pPS_Identifiers->hList_Combos;
		const CUtlVector< SimpleCombo* > &hCombo_VS = cfg->pVS_Identifiers->hList_Combos;

		const bool bPreviewMode = cfg->bPreviewMode;

		SHADOW_STATE
		{
#ifdef SHADER_EDITOR_DLL_SWARM
			pContext->flLightmapScaleFactor = pShaderShadow->GetLightMapScaleFactor();
#endif
			pShaderShadow->SetDefaultState();

			pShaderShadow->EnableDepthWrites( cfg->iDepthwritemode == DEPTHWRITE_NORMAL );
			pShaderShadow->EnableDepthTest( cfg->iDepthtestmode == DEPTHTEST_NORMAL );

			switch (cfg->iCullmode)
			{
			case CULLMODE_CW:
					pShaderShadow->EnableCulling( true );
					pShaderShadow->PolyMode( SHADER_POLYMODEFACE_FRONT, SHADER_POLYMODE_FILL );
				break;
			case CULLMODE_CCW:
					pShaderShadow->EnableCulling( true );
					pShaderShadow->PolyMode( SHADER_POLYMODEFACE_BACK, SHADER_POLYMODE_FILL );
				break;
			case CULLMODE_NONE:
					pShaderShadow->EnableCulling( false );
					pShaderShadow->PolyMode( SHADER_POLYMODEFACE_FRONT_AND_BACK, SHADER_POLYMODE_FILL );
				break;
			}

			switch (cfg->iAlphablendmode)
			{
			default:
					pShaderShadow->EnableAlphaWrites( true );
				break;
			case ABLEND_SIMPLE:
					EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
				break;
			case ABLEND_SIMPLE_INVERTED:
					EnableAlphaBlending( SHADER_BLEND_ONE_MINUS_SRC_ALPHA, SHADER_BLEND_SRC_ALPHA );
				break;
			case ABLEND_ALPHATEST:
					pShaderShadow->EnableAlphaTest( true );
					pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, cfg->flAlphaTestRef );
				break;
			case ABLEND_ADDITIVE:
#ifdef SHADER_EDITOR_DLL_2006
					EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_DST_ALPHA );
#else
					SetBlendingShadowState( BT_ADD );
#endif
					pShaderShadow->EnableBlending( true );
				break;
#ifndef SHADER_EDITOR_DLL_2006
			case ABLEND_ALPHA2COVERAGE:
					pShaderShadow->EnableAlphaTest( true );
					pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, 0.5f );
					pShaderShadow->EnableAlphaToCoverage( true );
				break;
#endif
			}

#ifdef SHADER_EDITOR_DLL_SWARM
			for ( int i = 0; i < hTextures_VS.Count(); i++ )
			{
				SimpleTexture *_tex = hTextures_VS[i];
				pShaderShadow->EnableVertexTexture( VertexTextureSampler_t(_tex->iSamplerIndex), true );
			}
#endif

			for ( int i = 0; i < hTextures_PS.Count(); i++ )
			{
				SimpleTexture *_tex = hTextures_PS[i];

				bool bDoSRGBRead = _tex->bSRGB;
				pShaderShadow->EnableTexture( Sampler_t(_tex->iSamplerIndex), true );

				switch ( _tex->iTextureMode )
				{
#ifndef SHADER_EDITOR_DLL_2006
				case HLSLTEX_FLASHLIGHT_DEPTH:
						if ( UsingFlashlight( params ) )
							pShaderShadow->SetShadowDepthFiltering( Sampler_t(_tex->iSamplerIndex) );
					break;
#endif
				case HLSLTEX_LIGHTMAP:
				case HLSLTEX_LIGHTMAP_BUMPMAPPED:
						bDoSRGBRead = g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE;
					break;
				}

				pShaderShadow->EnableSRGBRead( Sampler_t(_tex->iSamplerIndex), bDoSRGBRead );
			}

			const int fmt = cfg->iVFMT_flags;

			VERTEXSHADERVERTEXFORMAT(	fmt,
										cfg->iVFMT_numTexcoords,
										cfg->iVFMT_texDim,
										cfg->iVFMT_numUserData );

			int index_vs_static = CalcShaderIndex( this, pShaderShadow, pShaderAPI, vertexCompression, params, hCombo_VS,
				true, cfg->pVS_Identifiers->inum_DynamicCombos, bPreviewMode );
			int index_ps_static = CalcShaderIndex( this, pShaderShadow, pShaderAPI, vertexCompression, params, hCombo_PS,
				true, cfg->pPS_Identifiers->inum_DynamicCombos, bPreviewMode );

			bool bEnableFog = false;

			for ( int i = 0; i < hCombo_PS.Count(); i++ )
			{
				if ( hCombo_PS[i]->iComboType == HLSLCOMBO_PIXELFOG )
					bEnableFog = true;
			}

			pShaderShadow->EnableSRGBWrite( cfg->bsRGBWrite );

			if ( UsingFlashlight( params ) )
			{
				FogToBlack();
				if ( cfg->iAlphablendmode == ABLEND_ALPHATEST )
				{
					pShaderShadow->EnableAlphaTest( false );
					pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );
				}

				//pShaderShadow->EnableCulling( true );
				//pShaderShadow->PolyMode( SHADER_POLYMODEFACE_FRONT, SHADER_POLYMODE_FILL );
				//SetAdditiveBlendingShadowState( BASETEXTURE, true );


				pShaderShadow->EnableDepthTest( true );
				pShaderShadow->EnableDepthWrites( false );

#ifdef SHADER_EDITOR_DLL_2006
				EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_DST_ALPHA );
#else
				SetBlendingShadowState( BT_ADD );
#endif
				pShaderShadow->EnableBlending( true );

				pShaderShadow->EnableAlphaWrites( false );
			}
			else if ( bEnableFog )
			{
				DefaultFog();
			}
			else
				DisableFog();

#if 0 //def SHADER_EDITOR_DLL_SWARM
			char tmp_vs[MAX_PATH*4];
			char tmp_ps[MAX_PATH*4];
			Q_snprintf( tmp_vs, sizeof( tmp_vs ), "%s", cfg->ProcVSName );
			Q_snprintf( tmp_ps, sizeof( tmp_ps ), "%s", cfg->ProcPSName );
			ShaderNameToSwarmUnique( tmp_vs, sizeof( tmp_vs ) );
			ShaderNameToSwarmUnique( tmp_ps, sizeof( tmp_ps ) );
			pShaderShadow->SetVertexShader( tmp_vs, index_vs_static );
			pShaderShadow->SetPixelShader( tmp_ps, index_ps_static );
#else
#ifdef _WIN32
			pShaderShadow->SetVertexShader( cfg->ProcVSName, index_vs_static );
			pShaderShadow->SetPixelShader( cfg->ProcPSName, index_ps_static );
#else // POSIX
			char tmp_vs[MAX_PATH*4];
			char tmp_ps[MAX_PATH*4];
			V_strncpy(tmp_vs, cfg->ProcVSName, sizeof( tmp_vs ) );
			V_strncpy(tmp_ps, cfg->ProcPSName, sizeof( tmp_ps ) );
			//Msg( "VertexShader name: %s.\n", tmp_vs );
			//Msg( "PixelShader name: %s.\n", tmp_ps );
			char * tmp_p_vs;
			char * tmp_p_ps;
			tmp_p_vs = V_strstr( tmp_vs, "_vs30" );
			tmp_p_ps = V_strstr( tmp_ps, "_ps30" );
			V_strncpy( tmp_p_vs, "_vs20", 7 );
			V_strncpy( tmp_p_ps, "_ps20b", 7 );
			//Msg( "PS 2.0 VertexShader name: %s.\n", tmp_vs );
			//Msg( "PS 2.0 PixelShader name: %s.\n", tmp_ps );
			// load ps2.0 shaders
			pShaderShadow->SetVertexShader( tmp_vs, index_vs_static );
			pShaderShadow->SetPixelShader( tmp_ps, index_ps_static );
#endif // POSIX
#endif
		}

		DYNAMIC_STATE
		{
#ifdef SHADER_EDITOR_DLL_SWARM
			PI_BeginCommandBuffer();
#endif
			pShaderAPI->SetDefaultState();

			int index_vs_dynamic = CalcShaderIndex( this, pShaderShadow, pShaderAPI, vertexCompression, params, hCombo_VS,
				false, cfg->pVS_Identifiers->inum_DynamicCombos, bPreviewMode );
			int index_ps_dynamic = CalcShaderIndex( this, pShaderShadow, pShaderAPI, vertexCompression, params, hCombo_PS,
				false, cfg->pPS_Identifiers->inum_DynamicCombos, bPreviewMode );

			Assert( bCustomShader || index_vs_dynamic < 2 );

			pShaderAPI->SetVertexShaderIndex( index_vs_dynamic );
			pShaderAPI->SetPixelShaderIndex( index_ps_dynamic );

			int texCount = hTextures_PS.Count();
			SimpleTexture *const*_texBase = hTextures_PS.Base();
			SimpleTexture *_tex = NULL;

			for ( ; texCount > 0; texCount--, _texBase++ )
			{
				_tex = *_texBase;

				Assert( _tex != NULL && hTextures_PS.HasElement( _tex ) );

				int index = TEXTURE_0 + _tex->iSamplerIndex;
				int fixed_index = GetParamIdxFromMode( _tex->iTextureMode );

				if ( bCustomShader && fixed_index >= 0 )
					index = fixed_index;

				BindTextureByAutoType( !bCustomShader, pShaderAPI, this,
					_tex->iTextureMode,
					SHADER_SAMPLER0 + _tex->iSamplerIndex,
					index );
			}

			texCount = hTextures_VS.Count();
			_texBase = hTextures_VS.Base();

			for ( ; texCount > 0; texCount--, _texBase++ )
			{
				_tex = *_texBase;

				Assert( _tex != NULL && hTextures_VS.HasElement( _tex ) );

				int index = TEXTURE_VS_0 + _tex->iSamplerIndex;
				int fixed_index = GetParamIdxFromMode( _tex->iTextureMode );

				if ( bCustomShader && fixed_index >= 0 )
					index = fixed_index;

#ifndef SHADER_EDITOR_DLL_2006
				BindTextureByAutoType( !bCustomShader, pShaderAPI, this,
					_tex->iTextureMode,
					SHADER_VERTEXTEXTURE_SAMPLER0 + _tex->iSamplerIndex,
					index, -1, false );
#endif
			}

			int constCount = hConst_PS.Count();
			SimpleEnvConstant *const*_constBase = hConst_PS.Base();
			SimpleEnvConstant *_const = NULL;

			for ( ; constCount > 0; constCount--, _constBase++ )
			{
				_const = *_constBase;
				UpdateConstantByIdentifier( this, pShaderAPI, params, _const, pContext, true, MUTABLE_01, VPSTATIC_00 );
			}

			constCount = hConst_VS.Count();
			_constBase = hConst_VS.Base();

			for ( ; constCount > 0; constCount--, _constBase++ )
			{
				_const = *_constBase;
				UpdateConstantByIdentifier( this, pShaderAPI, params, _const, pContext, false, MUTABLE_01, VPSTATIC_00 );
			}

#ifdef SHADER_EDITOR_DLL_SWARM
			PI_EndCommandBuffer();
#endif
		}

#ifdef SHADER_EDITOR_DLL_2006
		Draw();
#else
		bool bDraw = true;
#if !defined(SHADER_EDITOR_DLL_SWARM)
		if ( !IS_FLAG2_SET( MATERIAL_VAR2_SUPPORTS_FLASHLIGHT ) && UsingFlashlight( params ) && pShaderAPI )
			bDraw = false;
#endif
		Draw( bDraw );
#endif

	}
END_SHADER
