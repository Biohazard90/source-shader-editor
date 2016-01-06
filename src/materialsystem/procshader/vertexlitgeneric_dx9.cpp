//========= Copyright � 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=====================================================================================//

#include "BaseVSShader.h"
#include "vertexlitgeneric_dx9_helper.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER( SDK_VertexLitGeneric, "Help for SDK_VertexLitGeneric" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( ALBEDO, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "albedo (Base texture with no baked lighting)" )
		SHADER_PARAM( COMPRESS, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "compression wrinklemap" )
		SHADER_PARAM( STRETCH, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "expansion wrinklemap" )
		SHADER_PARAM( SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint" )
		SHADER_PARAM( DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture" )
		SHADER_PARAM( DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail" )
		SHADER_PARAM( DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture" )
		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap" )
		SHADER_PARAM( ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "envmap frame number" )
		SHADER_PARAM( ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "envmap mask" )
		SHADER_PARAM( ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( ENVMAPMASKTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$envmapmask texcoord transform" )
		SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint" )
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map" )
		SHADER_PARAM( BUMPCOMPRESS, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader3_normal", "compression bump map" )
		SHADER_PARAM( BUMPSTRETCH, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "expansion bump map" )
		SHADER_PARAM( BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap" )
		SHADER_PARAM( BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform" )
		SHADER_PARAM( ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
		SHADER_PARAM( ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal" )
 	    SHADER_PARAM( SELFILLUM_ENVMAPMASK_ALPHA, SHADER_PARAM_TYPE_FLOAT,"0.0","defines that self illum value comes from env map mask alpha" )
		SHADER_PARAM( SELFILLUMFRESNEL, SHADER_PARAM_TYPE_BOOL, "0", "Self illum fresnel" )
		SHADER_PARAM( SELFILLUMFRESNELMINMAXEXP, SHADER_PARAM_TYPE_VEC4, "0", "Self illum fresnel min, max, exp" )
		SHADER_PARAM( ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "" )	
		SHADER_PARAM( FLASHLIGHTNOLAMBERT, SHADER_PARAM_TYPE_BOOL, "0", "Flashlight pass sets N.L=1.0" )

		// Debugging term for visualizing ambient data on its own
		SHADER_PARAM( AMBIENTONLY, SHADER_PARAM_TYPE_INTEGER, "0", "Control drawing of non-ambient light ()" )

		SHADER_PARAM( PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "5.0", "Phong exponent for local specular lights" )
		SHADER_PARAM( PHONGTINT, SHADER_PARAM_TYPE_VEC3, "5.0", "Phong tint for local specular lights" )
		SHADER_PARAM( PHONGALBEDOTINT, SHADER_PARAM_TYPE_BOOL, "1.0", "Apply tint by albedo (controlled by spec exponent texture" )
		SHADER_PARAM( LIGHTWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "1D ramp texture for tinting scalar diffuse term" )
		SHADER_PARAM( PHONGWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "warp the specular term" )
		SHADER_PARAM( PHONGFRESNELRANGES, SHADER_PARAM_TYPE_VEC3, "[0  0.5  1]", "Parameters for remapping fresnel output" )
		SHADER_PARAM( PHONGBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Phong overbrightening factor (specular mask channel should be authored to account for this)" )
		SHADER_PARAM( PHONGEXPONENTTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Phong Exponent map" )
		SHADER_PARAM( PHONG, SHADER_PARAM_TYPE_BOOL, "0", "enables phong lighting" )
		SHADER_PARAM( BASEMAPALPHAPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "indicates that there is no normal map and that the phong mask is in base alpha" )
		SHADER_PARAM( INVERTPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "invert the phong mask (0=full phong, 1=no phong)" )
		SHADER_PARAM( ENVMAPFRESNEL, SHADER_PARAM_TYPE_FLOAT, "0", "Degree to which Fresnel should be applied to env map" )
		SHADER_PARAM( SELFILLUMMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "If we bind a texture here, it overrides base alpha (if any) for self illum" )

	    // detail (multi-) texturing
	    SHADER_PARAM( DETAILBLENDMODE, SHADER_PARAM_TYPE_INTEGER, "0", "mode for combining detail texture with base. 0=normal, 1= additive, 2=alpha blend detail over base, 3=crossfade" )
		SHADER_PARAM( DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture." )
		SHADER_PARAM( DETAILTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "detail texture tint" )
		SHADER_PARAM( DETAILTEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$detail texcoord transform" )

		// Rim lighting terms
		SHADER_PARAM( RIMLIGHT, SHADER_PARAM_TYPE_BOOL, "0", "enables rim lighting" )
		SHADER_PARAM( RIMLIGHTEXPONENT, SHADER_PARAM_TYPE_FLOAT, "4.0", "Exponent for rim lights" )
		SHADER_PARAM( RIMLIGHTBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Boost for rim lights" )
		SHADER_PARAM( RIMMASK, SHADER_PARAM_TYPE_BOOL, "0", "Indicates whether or not to use alpha channel of exponent texture to mask the rim term" )

	    // Seamless mapping scale
		SHADER_PARAM( SEAMLESS_BASE, SHADER_PARAM_TYPE_BOOL, "0", "whether to apply seamless mapping to the base texture. requires a smooth model." )
		SHADER_PARAM( SEAMLESS_DETAIL, SHADER_PARAM_TYPE_BOOL, "0", "where to apply seamless mapping to the detail texture." )
	    SHADER_PARAM( SEAMLESS_SCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "the scale for the seamless mapping. # of repetions of texture per inch." )

		// Emissive Scroll Pass
		SHADER_PARAM( EMISSIVEBLENDENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable emissive blend pass" )
		SHADER_PARAM( EMISSIVEBLENDBASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map" )
		SHADER_PARAM( EMISSIVEBLENDSCROLLVECTOR, SHADER_PARAM_TYPE_VEC2, "[0.11 0.124]", "Emissive scroll vec" )
		SHADER_PARAM( EMISSIVEBLENDSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "Emissive blend strength" )
		SHADER_PARAM( EMISSIVEBLENDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map" )
		SHADER_PARAM( EMISSIVEBLENDTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint" )
		SHADER_PARAM( EMISSIVEBLENDFLOWTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "flow map" )
		SHADER_PARAM( TIME, SHADER_PARAM_TYPE_FLOAT, "0.0", "Needs CurrentTime Proxy" )

		// Cloak Pass
		SHADER_PARAM( CLOAKPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass" )
		SHADER_PARAM( CLOAKFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "" )
		SHADER_PARAM( CLOAKCOLORTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Cloak color tint" )
		SHADER_PARAM( REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "" )

		// Flesh Interior Pass
		SHADER_PARAM( FLESHINTERIORENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable Flesh interior blend pass" )
		SHADER_PARAM( FLESHINTERIORTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh color texture" )
		SHADER_PARAM( FLESHINTERIORNOISETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh noise texture" )
		SHADER_PARAM( FLESHBORDERTEXTURE1D, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh border 1D texture" )
		SHADER_PARAM( FLESHNORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh normal texture" )
		SHADER_PARAM( FLESHSUBSURFACETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh subsurface texture" )
		SHADER_PARAM( FLESHCUBETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh cubemap texture" )
		SHADER_PARAM( FLESHBORDERNOISESCALE, SHADER_PARAM_TYPE_FLOAT, "1.5", "Flesh Noise UV scalar for border" )
		SHADER_PARAM( FLESHDEBUGFORCEFLESHON, SHADER_PARAM_TYPE_BOOL, "0", "Flesh Debug full flesh" )
		SHADER_PARAM( FLESHEFFECTCENTERRADIUS1, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
		SHADER_PARAM( FLESHEFFECTCENTERRADIUS2, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
		SHADER_PARAM( FLESHEFFECTCENTERRADIUS3, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
		SHADER_PARAM( FLESHEFFECTCENTERRADIUS4, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
		SHADER_PARAM( FLESHSUBSURFACETINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Subsurface Color" )
		SHADER_PARAM( FLESHBORDERWIDTH, SHADER_PARAM_TYPE_FLOAT, "0.3", "Flesh border" )
		SHADER_PARAM( FLESHBORDERSOFTNESS, SHADER_PARAM_TYPE_FLOAT, "0.42", "Flesh border softness (> 0.0 && <= 0.5)" )
		SHADER_PARAM( FLESHBORDERTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Flesh border Color" )
		SHADER_PARAM( FLESHGLOBALOPACITY, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh global opacity" )
		SHADER_PARAM( FLESHGLOSSBRIGHTNESS, SHADER_PARAM_TYPE_FLOAT, "0.66", "Flesh gloss brightness" )
		SHADER_PARAM( FLESHSCROLLSPEED, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh scroll speed" )

		SHADER_PARAM( SEPARATEDETAILUVS, SHADER_PARAM_TYPE_BOOL, "0", "Use texcoord1 for detail texture" )
		SHADER_PARAM( LINEARWRITE, SHADER_PARAM_TYPE_INTEGER, "0", "Disables SRGB conversion of shader results." )
		SHADER_PARAM( DEPTHBLEND, SHADER_PARAM_TYPE_INTEGER, "0", "fade at intersection boundaries. Only supported without bumpmaps" )
		SHADER_PARAM( DEPTHBLENDSCALE, SHADER_PARAM_TYPE_FLOAT, "50.0", "Amplify or reduce DEPTHBLEND fading. Lower values make harder edges." )
	END_SHADER_PARAMS

	void SetupVars( VertexLitGeneric_DX9_Vars_t& info )
	{
		info.m_nBaseTexture = BASETEXTURE;
		info.m_nWrinkle = COMPRESS;
		info.m_nStretch = STRETCH;
		info.m_nBaseTextureFrame = FRAME;
		info.m_nBaseTextureTransform = BASETEXTURETRANSFORM;
		info.m_nAlbedo = ALBEDO;
		info.m_nSelfIllumTint = SELFILLUMTINT;
		info.m_nDetail = DETAIL;
		info.m_nDetailFrame = DETAILFRAME;
		info.m_nDetailScale = DETAILSCALE;
		info.m_nEnvmap = ENVMAP;
		info.m_nEnvmapFrame = ENVMAPFRAME;
		info.m_nEnvmapMask = ENVMAPMASK;
		info.m_nEnvmapMaskFrame = ENVMAPMASKFRAME;
		info.m_nEnvmapMaskTransform = ENVMAPMASKTRANSFORM;
		info.m_nEnvmapTint = ENVMAPTINT;
		info.m_nBumpmap = BUMPMAP;
		info.m_nNormalWrinkle = BUMPCOMPRESS;
		info.m_nNormalStretch = BUMPSTRETCH;
		info.m_nBumpFrame = BUMPFRAME;
		info.m_nBumpTransform = BUMPTRANSFORM;
		info.m_nEnvmapContrast = ENVMAPCONTRAST;
		info.m_nEnvmapSaturation = ENVMAPSATURATION;
		info.m_nAlphaTestReference = ALPHATESTREFERENCE;
		info.m_nFlashlightNoLambert = FLASHLIGHTNOLAMBERT;

		info.m_nFlashlightTexture = FLASHLIGHTTEXTURE;
		info.m_nFlashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
		info.m_nSelfIllumEnvMapMask_Alpha = SELFILLUM_ENVMAPMASK_ALPHA;
		info.m_nSelfIllumFresnel = SELFILLUMFRESNEL;
		info.m_nSelfIllumFresnelMinMaxExp = SELFILLUMFRESNELMINMAXEXP;

		info.m_nAmbientOnly = AMBIENTONLY;
		info.m_nPhongExponent = PHONGEXPONENT;
		info.m_nPhongExponentTexture = PHONGEXPONENTTEXTURE;
		info.m_nPhongTint = PHONGTINT;
		info.m_nPhongAlbedoTint = PHONGALBEDOTINT;
		info.m_nDiffuseWarpTexture = LIGHTWARPTEXTURE;
		info.m_nPhongWarpTexture = PHONGWARPTEXTURE;
		info.m_nPhongBoost = PHONGBOOST;
		info.m_nPhongFresnelRanges = PHONGFRESNELRANGES;
		info.m_nPhong = PHONG;
		info.m_nBaseMapAlphaPhongMask = BASEMAPALPHAPHONGMASK;
		info.m_nEnvmapFresnel = ENVMAPFRESNEL;
		info.m_nDetailTextureCombineMode = DETAILBLENDMODE;
		info.m_nDetailTextureBlendFactor = DETAILBLENDFACTOR;
		info.m_nDetailTextureTransform = DETAILTEXTURETRANSFORM;

		// Rim lighting parameters
		info.m_nRimLight = RIMLIGHT;
		info.m_nRimLightPower = RIMLIGHTEXPONENT;
		info.m_nRimLightBoost = RIMLIGHTBOOST;
		info.m_nRimMask = RIMMASK;

		// seamless
		info.m_nSeamlessScale = SEAMLESS_SCALE;
		info.m_nSeamlessDetail = SEAMLESS_DETAIL;
		info.m_nSeamlessBase = SEAMLESS_BASE;

		info.m_nSeparateDetailUVs = SEPARATEDETAILUVS;

		info.m_nLinearWrite = LINEARWRITE;
		info.m_nDetailTint = DETAILTINT;
		info.m_nInvertPhongMask = INVERTPHONGMASK;

		info.m_nDepthBlend = DEPTHBLEND;
		info.m_nDepthBlendScale = DEPTHBLENDSCALE;

		info.m_nSelfIllumMask = SELFILLUMMASK;
	}

	bool NeedsPowerOfTwoFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame ) const 
	{ 
		if ( params[CLOAKPASSENABLED]->GetIntValue() ) // If material supports cloaking
		{
			if ( bCheckSpecificToThisFrame == false ) // For setting model flag at load time
				return true;
			else if ( ( params[CLOAKFACTOR]->GetFloatValue() > 0.0f ) && ( params[CLOAKFACTOR]->GetFloatValue() < 1.0f ) ) // Per-frame check
				return true;
			// else, not cloaking this frame, so check flag2 in case the base material still needs it
		}

		// Check flag2 if not drawing cloak pass
		return IS_FLAG2_SET( MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE ); 
	}

	bool IsTranslucent( IMaterialVar **params ) const
	{
		if ( params[CLOAKPASSENABLED]->GetIntValue() ) // If material supports cloaking
		{
			if ( ( params[CLOAKFACTOR]->GetFloatValue() > 0.0f ) && ( params[CLOAKFACTOR]->GetFloatValue() < 1.0f ) ) // Per-frame check
				return true;
			// else, not cloaking this frame, so check flag in case the base material still needs it
		}

		// Check flag if not drawing cloak pass
		return IS_FLAG_SET( MATERIAL_VAR_TRANSLUCENT ); 
	}

	SHADER_INIT_PARAMS()
	{
		VertexLitGeneric_DX9_Vars_t vars;
		SetupVars( vars );
		InitParamsVertexLitGeneric_DX9( this, params, pMaterialName, true, vars );

		{
			params[CLOAKPASSENABLED]->SetIntValue( 0 );
		}



		{
			params[EMISSIVEBLENDENABLED]->SetIntValue( 0 );
		}


		{
			params[FLESHINTERIORENABLED]->SetIntValue( 0 );
		}
	}

	SHADER_FALLBACK
	{

		return 0;
	}

	SHADER_INIT
	{
		VertexLitGeneric_DX9_Vars_t vars;
		SetupVars( vars );
		InitVertexLitGeneric_DX9( this, params, true, vars );
	}

	SHADER_DRAW
	{
		// Skip the standard rendering if cloak pass is fully opaque
		bool bDrawStandardPass = true;
		if ( params[CLOAKPASSENABLED]->GetIntValue() && ( pShaderShadow == NULL ) ) // && not snapshotting
		{
		}

		// Standard rendering pass
		if ( bDrawStandardPass )
		{
			VertexLitGeneric_DX9_Vars_t vars;
			SetupVars( vars );
			DrawVertexLitGeneric_DX9( this, params, pShaderAPI, pShaderShadow, true, vars, vertexCompression, pContextDataPtr );
		}
		else
		{
			// Skip this pass!
			Draw( false );
		}

		// Cloak Pass
		if ( params[CLOAKPASSENABLED]->GetIntValue() )
		{

		}

		// Emissive Scroll Pass
		if ( params[EMISSIVEBLENDENABLED]->GetIntValue() )
		{

		}

		// Flesh Interior Pass
		if ( params[FLESHINTERIORENABLED]->GetIntValue() )
		{

		}
	}
END_SHADER
