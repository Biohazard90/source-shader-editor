
#include "shaderincludes.h"
#include "convar.h"

#include "fallback_vs20.inc"
#include "fallback_ps20.inc"

#include "../ProcShaderInterface.h"

#include "node_ps20.inc"
#include "node_vs20.inc"

#include "psin_ps20.inc"
#include "psin_vs20.inc"

bool IsTextypeUsingCustomTexture( int textype )
{
	if ( textype == HLSLTEX_CUSTOMPARAM ||
		textype == HLSLTEX_BASETEXTURE ||
		textype == HLSLTEX_BUMPMAP ||
		textype == HLSLTEX_ENVMAP ) //||

		//textype == HLSLTEX_FLASHLIGHT_COOKIE )
		return true;
	return false;
}

void BindVSTexture( IShaderDynamicAPI *pShaderAPI, CBaseVSShader *pShader,
	int sampler, int TextureVar, int FrameVar )
{
#ifdef SHADER_EDITOR_DLL_SWARM
	pShader->BindVertexTexture( (VertexTextureSampler_t)(sampler), TextureVar, FrameVar );
#elif SHADER_EDITOR_DLL_2006
#else
	sampler = 0;
	StandardTextureId_t texID = (StandardTextureId_t)(STANDARDTEX_USER_FIRST_INDEX + sampler);

	ShaderAPITextureHandle_t hHandle = pShader->GetShaderAPITextureBindHandle( TextureVar, FrameVar );
	pShaderAPI->SetStandardTextureHandle( texID, hHandle );
	pShaderAPI->BindStandardVertexTexture( (VertexTextureSampler_t)(sampler), texID );
#endif
}

void BindTextureByAutoType( bool bPreview, IShaderDynamicAPI *pShaderAPI, CBaseVSShader *pShader,
							int type, int sampleridx, int TextureVar, int FrameVar, bool bPS )
{
	if ( IsTextypeUsingCustomTexture(type) )
	{
		if ( bPS )
		{
#ifdef SHADER_EDITOR_DLL_SWARM
			if ( type == HLSLTEX_ENVMAP && !bPreview )
				pShaderAPI->BindStandardTexture( Sampler_t(sampleridx), TEXTURE_LOCAL_ENV_CUBEMAP );
			else
#endif
				pShader->BindTexture( Sampler_t(sampleridx), TextureVar );
		}
		else
			BindVSTexture( pShaderAPI, pShader, sampleridx, TextureVar, FrameVar );
	}
	else
	{
		if ( bPreview )
		{
			switch ( type )
			{
			case HLSLTEX_LIGHTMAP:
			case HLSLTEX_LIGHTMAP_BUMPMAPPED:
				type = HLSLTEX_GREY;
				break;
			}
		}

		StandardTextureId_t tex;
		switch ( type )
		{
		case HLSLTEX_LIGHTMAP:
				tex = TEXTURE_LIGHTMAP;
			break;
		case HLSLTEX_LIGHTMAP_BUMPMAPPED:
				tex = TEXTURE_LIGHTMAP_BUMPED;
			break;
		default:
		case HLSLTEX_FRAMEBUFFER:
				tex = TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0;
			break;
		case HLSLTEX_BLACK:
				tex = TEXTURE_BLACK;
			break;
		case HLSLTEX_WHITE:
				tex = TEXTURE_WHITE;
			break;
		case HLSLTEX_GREY:
				tex = TEXTURE_GREY;
			break;
		case HLSLTEX_FLASHLIGHT_COOKIE:
			{
				VMatrix worldToTexture;
				FlashlightState_t state = pShaderAPI->GetFlashlightState( worldToTexture );
				pShader->BindTexture( Sampler_t(sampleridx), state.m_pSpotlightTexture, state.m_nSpotlightTextureFrame );
			}
			return;
#ifndef SHADER_EDITOR_DLL_2006
		case HLSLTEX_FLASHLIGHT_DEPTH:
			{
				Assert( bPS );

				VMatrix worldToTexture;
				ITexture *pFlashlightDepthTexture;
				FlashlightState_t state = pShaderAPI->GetFlashlightStateEx( worldToTexture, &pFlashlightDepthTexture );
				if ( state.m_bEnableShadows && pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() )
				{
					pShader->BindTexture( Sampler_t(sampleridx), pFlashlightDepthTexture, 0 );
					return;
				}
				tex = TEXTURE_WHITE;
			}
			break;
		case HLSLTEX_FLASHLIGHT_RANDOM:
				tex = TEXTURE_SHADOW_NOISE_2D;
			break;
		case HLSLTEX_MORPH:
				tex = TEXTURE_MORPH_ACCUMULATOR;
				if ( !pShaderAPI->IsHWMorphingEnabled() )
					return;
			break;
#endif
		}

		if ( bPS )
			pShaderAPI->BindStandardTexture( Sampler_t(sampleridx), tex );
#ifndef SHADER_EDITOR_DLL_2006
		else
			pShaderAPI->BindStandardVertexTexture( (VertexTextureSampler_t)(sampleridx), tex );
#endif
	}
}

BEGIN_VS_SHADER( NODE_CALC, "" )
	BEGIN_SHADER_PARAMS

		SHADER_PARAM( TEXTURE_RT, SHADER_PARAM_TYPE_TEXTURE, "", "" )

		SHADER_PARAM( INPUT_UVS_0, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( INPUT_UVS_1, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( INPUT_UVS_2, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( INPUT_UVS_3, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( INPUT_UVS_4, SHADER_PARAM_TYPE_VEC4, "", "" )
		SHADER_PARAM( INPUT_UVS_5, SHADER_PARAM_TYPE_VEC4, "", "" )

		SHADER_PARAM( OPERATION, SHADER_PARAM_TYPE_INTEGER, "", "" )

		SHADER_PARAM( CUSTOMTEXTURE, SHADER_PARAM_TYPE_STRING, "", "" )

		SHADER_PARAM( ISCUBEMAP, SHADER_PARAM_TYPE_BOOL, "", "" )
		SHADER_PARAM( SRGB, SHADER_PARAM_TYPE_BOOL, "", "" )
		SHADER_PARAM( TEXTURETYPE, SHADER_PARAM_TYPE_INTEGER, "", "" )

	END_SHADER_PARAMS

	SHADER_INIT
	{
		params[TEXTURE_RT]->SetStringValue( "_rt_SEdit_pingpong_0" );
		LoadTexture( TEXTURE_RT );

		if ( params[CUSTOMTEXTURE]->IsDefined() )
		{
			bool bCubemap = !!params[ISCUBEMAP]->GetIntValue();

			if ( bCubemap )
				LoadCubeMap(CUSTOMTEXTURE);
			else
				LoadTexture(CUSTOMTEXTURE);
		}
	}
	
	SHADER_FALLBACK
	{
		return 0;
	}
	
	SHADER_DRAW
	{
		const int command = params[ OPERATION ]->GetIntValue();

		const int texType = params[ TEXTURETYPE ]->GetIntValue();

		const bool bEnableTexture_1 =	command == NPSOP_TEXTURE_LOOKUP_2D ||
										command == NPSOP_TEXTURE_LOOKUP_3D ||
										command == NPSOP_TEXTURE_LOOKUP_FIXED;

		SHADOW_STATE
		{
			pShaderShadow->SetDefaultState();

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, false );
			if ( bEnableTexture_1 )
			{
				const bool bSRGB = !!params[ SRGB ]->GetIntValue();
				pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, bSRGB );
			}

			pShaderShadow->EnableSRGBWrite( false );

			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( false );
			pShaderShadow->EnableCulling( false );

			int fmt = VERTEX_POSITION;
			VERTEXSHADERVERTEXFORMAT( fmt, 1, 0, 0 );

			//if ( g_pHardwareConfig->SupportsShaderModel_3_0() )
			//{
			//	pShaderShadow->SetVertexShader( "node_vs30", 0 );
			//	pShaderShadow->SetPixelShader( "node_ps30", command );
			//}
			//else
			{
				pShaderShadow->SetVertexShader( "node_vs20", 0 );
				//if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
				//	pShaderShadow->SetPixelShader( "node_ps20b", command );
				//else
					pShaderShadow->SetPixelShader( "node_ps20", command );
			}

			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableBlending( false );
		}

		DYNAMIC_STATE
		{
#ifdef SHADER_EDITOR_DLL_SWARM
			PI_BeginCommandBuffer();
#endif
			pShaderAPI->SetDefaultState();

			pShaderAPI->SetVertexShaderIndex();
			pShaderAPI->SetPixelShaderIndex();

			BindTexture( SHADER_SAMPLER0, TEXTURE_RT );
			if ( bEnableTexture_1 )
				BindTextureByAutoType( true, pShaderAPI, this, texType, SHADER_SAMPLER1, CUSTOMTEXTURE );

			for ( int i = 0; i < 6; i++ )
			{
				float uvs[4] = { 0, 0, 0, 0 };
				params[INPUT_UVS_0+i]->GetVecValue( uvs, 4 );
				pShaderAPI->SetPixelShaderConstant( i, uvs );
			}

			float info[4] = { 0, 0, 0, 0 };
			info[ 0 ] = gProcShaderCTRL->GetNormalizedPuzzleDelta();
			info[ 1 ] = 1.0f - gProcShaderCTRL->GetNormalizedPuzzleDelta();
			pShaderAPI->SetVertexShaderConstant( 48, info );
#ifdef SHADER_EDITOR_DLL_SWARM
			PI_EndCommandBuffer();
#endif
		}

#ifdef SHADER_EDITOR_DLL_SWARM
		Draw( true, false );
#else
		Draw();
#endif
	}
END_SHADER

BEGIN_VS_SHADER( NODE_PSIN, "" )
	BEGIN_SHADER_PARAMS

		SHADER_PARAM( VSNAME, SHADER_PARAM_TYPE_STRING, "", "" )
		SHADER_PARAM( OPERATION, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( VFMT_FLAGS, SHADER_PARAM_TYPE_INTEGER, "1", "" )
		SHADER_PARAM( VFMT_NUMTEXCOORDS, SHADER_PARAM_TYPE_INTEGER, "1", "" )
		SHADER_PARAM( VFMT_USERDATA, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( VFMT_TEXCOORDDIM_0, SHADER_PARAM_TYPE_INTEGER, "2", "" )
		SHADER_PARAM( VFMT_TEXCOORDDIM_1, SHADER_PARAM_TYPE_INTEGER, "2", "" )
		SHADER_PARAM( VFMT_TEXCOORDDIM_2, SHADER_PARAM_TYPE_INTEGER, "2", "" )

	END_SHADER_PARAMS

	SHADER_INIT
	{
		if ( !params[ VSNAME ]->IsDefined() )
			params[ VSNAME ]->SetStringValue( "psin_vs20" );
		if ( !params[ VFMT_FLAGS ]->GetIntValue() )
			params[ VFMT_FLAGS ]->SetUndefined();
	}
	
	SHADER_FALLBACK
	{
		return 0;
	}
	
	SHADER_DRAW
	{
		int command = params[ OPERATION ]->GetIntValue();

		BasicShaderCfg_t *cfg = gProcShaderCTRL->AccessVolatileData(1);

		SHADOW_STATE
		{
			pShaderShadow->SetDefaultState();
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( false );
			pShaderShadow->EnableCulling( false );
			pShaderShadow->EnableSRGBWrite( false );

			//int fmt = VERTEX_POSITION;
			//pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			int vfmt_flags = VERTEX_POSITION;
			int vfmt_num_t = 1;
			int vfmt_userdata = 0;
			int texcoordDim[3] = { 2, 2, 2 };

			if ( params[VFMT_FLAGS]->IsDefined() )
			{
				vfmt_flags = params[VFMT_FLAGS]->GetIntValue();
				vfmt_num_t = params[VFMT_NUMTEXCOORDS]->GetIntValue();
				vfmt_userdata = params[VFMT_USERDATA]->GetIntValue();

				//texcoordDim[ 0 ] = params[VFMT_TEXCOORDDIM_0]->GetIntValue();
				//texcoordDim[ 1 ] = params[VFMT_TEXCOORDDIM_1]->GetIntValue();
				//texcoordDim[ 2 ] = params[VFMT_TEXCOORDDIM_2]->GetIntValue();
			}

			VERTEXSHADERVERTEXFORMAT( vfmt_flags, vfmt_num_t, texcoordDim, vfmt_userdata );

			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableBlending( false );

			const char *szVSName = params[ VSNAME ]->GetStringValue();
			pShaderShadow->SetVertexShader( szVSName, 0 );

			if ( cfg && cfg->iShaderModel == SM_30 || Q_stristr( szVSName, "_vs30" ) != NULL )
				pShaderShadow->SetPixelShader( "psin_ps30", 0 );
			else
				pShaderShadow->SetPixelShader( "psin_ps20", 0 );
		}
		DYNAMIC_STATE
		{
#ifdef SHADER_EDITOR_DLL_SWARM
			PI_BeginCommandBuffer();
#endif
			pShaderAPI->SetDefaultState();

			pShaderAPI->SetVertexShaderIndex();
			pShaderAPI->SetPixelShaderIndex( command );

			if ( cfg )
			{
				CUtlVector< SimpleEnvConstant* > &hConst_VS = cfg->pVS_Identifiers->hList_EConstants;
				for ( int i = 0; i < hConst_VS.Count(); i++ )
				{
					SimpleEnvConstant *_const = hConst_VS[ i ];
					UpdateConstantByIdentifier( this, pShaderAPI, params, _const, NULL, false );
				}
			}
#ifdef SHADER_EDITOR_DLL_SWARM
			PI_EndCommandBuffer();
#endif
		}

#ifdef SHADER_EDITOR_DLL_SWARM
		Draw( true, false );
#else
		Draw();
#endif
	}
END_SHADER