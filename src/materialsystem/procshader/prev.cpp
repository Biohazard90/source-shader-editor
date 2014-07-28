
#include "shaderincludes.h"

#include "convar.h"

#include "prev_vs20.inc"
#include "prev_ps20.inc"

#include "skymask_vs20.inc"
#include "skymask_ps20.inc"


BEGIN_VS_SHADER( BACKGROUND_PREV, "" )
	BEGIN_SHADER_PARAMS

		SHADER_PARAM( ISCUBEMAP, SHADER_PARAM_TYPE_BOOL, "", "" )

	END_SHADER_PARAMS

	SHADER_INIT
	{
		if ( params[BASETEXTURE]->IsDefined() )
		{
			bool bCubemap = !!params[ISCUBEMAP]->GetIntValue();
			if ( bCubemap )
				LoadCubeMap(BASETEXTURE);
			else
				LoadTexture(BASETEXTURE);
		}
	}

	SHADER_FALLBACK
	{
		return 0;
	}
	
	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->SetDefaultState();
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( false );
			pShaderShadow->EnableSRGBWrite( false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

			VERTEXSHADERVERTEXFORMAT( VERTEX_POSITION | VERTEX_NORMAL, 1, 0, 0 );

			pShaderShadow->SetVertexShader( "prev_vs20", 0 );
			pShaderShadow->SetPixelShader( "prev_ps20", 0 );
		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();

			const bool bCubemap = params[ISCUBEMAP]->IsDefined() && params[ISCUBEMAP]->GetIntValue();

			pShaderAPI->SetVertexShaderIndex();
			pShaderAPI->SetPixelShaderIndex( bCubemap ? 1 : 0 );

			BindTexture( SHADER_SAMPLER0, BASETEXTURE );
		}
#ifdef SHADER_EDITOR_DLL_SWARM
		Draw( true, false );
#else
		Draw();
#endif
	}

END_SHADER


BEGIN_VS_SHADER( COLORPICKER_BLEND, "" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( COLOR_00, SHADER_PARAM_TYPE_VEC3, "", "" )
		SHADER_PARAM( COLOR_10, SHADER_PARAM_TYPE_VEC3, "", "" )
		SHADER_PARAM( COLOR_11, SHADER_PARAM_TYPE_VEC3, "", "" )
		SHADER_PARAM( COLOR_01, SHADER_PARAM_TYPE_VEC3, "", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
	}

	SHADER_FALLBACK
	{
		return 0;
	}
	
	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->SetDefaultState();
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( false );
			pShaderShadow->EnableSRGBWrite( false );

			VERTEXSHADERVERTEXFORMAT( VERTEX_POSITION, 1, 0, 0 );

			pShaderShadow->SetVertexShader( "colorpicker_blend_vs20", 0 );
			pShaderShadow->SetPixelShader( "colorpicker_blend_ps20", 0 );
		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();

			pShaderAPI->SetVertexShaderIndex();
			pShaderAPI->SetPixelShaderIndex( 0 );

			float data[4] = { 0, 0, 0, 0 };
			for ( int i = 0; i < 4; i++ )
			{
				params[ COLOR_00 + i ]->GetVecValue( data, 3 );
				pShaderAPI->SetPixelShaderConstant( i, data );
			}
		}
#ifdef SHADER_EDITOR_DLL_SWARM
		Draw( true, false );
#else
		Draw();
#endif
	}

END_SHADER




BEGIN_VS_SHADER( FILL_SKYMASK, "" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( COMBINEMODE, SHADER_PARAM_TYPE_BOOL, "", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
		if ( params[COMBINEMODE]->GetIntValue() )
			LoadTexture( BASETEXTURE );
	}

	SHADER_FALLBACK
	{
		return 0;
	}
	
	SHADER_DRAW
	{
		const bool bCombineMode = !!params[COMBINEMODE]->GetIntValue();

		SHADOW_STATE
		{
			pShaderShadow->SetDefaultState();

			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( false );
			pShaderShadow->EnableColorWrites( false );
			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableCulling( false );

			VERTEXSHADERVERTEXFORMAT( VERTEX_POSITION, 1, 0, 0 );

			if ( bCombineMode )
				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

			skymask_vs20_Static_Index index_vs;
			index_vs.SetDRAWWHITE( false );
			index_vs.SetCOMBINE( bCombineMode );
			skymask_ps20_Static_Index index_ps;
			index_ps.SetDRAWWHITE( false );
			index_ps.SetCOMBINE( bCombineMode );

			pShaderShadow->SetVertexShader( "skymask_vs20", index_vs.GetIndex() );
			pShaderShadow->SetPixelShader( "skymask_ps20", index_ps.GetIndex() );
		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();

			pShaderAPI->SetVertexShaderIndex();
			pShaderAPI->SetPixelShaderIndex( 0 );

			if ( bCombineMode )
				BindTexture( SHADER_SAMPLER0, BASETEXTURE );
		}
#ifdef SHADER_EDITOR_DLL_SWARM
		Draw( true, false );
#else
		Draw();
#endif


		SHADOW_STATE
		{
			pShaderShadow->SetDefaultState();

			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( true );
			pShaderShadow->EnableColorWrites( false );
			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableCulling( false );
			VERTEXSHADERVERTEXFORMAT( VERTEX_POSITION, 1, 0, 0 );

			if ( bCombineMode )
				pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );

			skymask_vs20_Static_Index index_vs;
			index_vs.SetDRAWWHITE( true );
			index_vs.SetCOMBINE( bCombineMode );
			skymask_ps20_Static_Index index_ps;
			index_ps.SetDRAWWHITE( true );
			index_ps.SetCOMBINE( bCombineMode );

			pShaderShadow->SetVertexShader( "skymask_vs20", index_vs.GetIndex() );
			pShaderShadow->SetPixelShader( "skymask_ps20", index_ps.GetIndex() );
		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();

			pShaderAPI->SetVertexShaderIndex();
			pShaderAPI->SetPixelShaderIndex( 0 );

			if ( bCombineMode )
				BindTexture( SHADER_SAMPLER0, BASETEXTURE );
		}
#ifdef SHADER_EDITOR_DLL_SWARM
		Draw( true, false );
#else
		Draw();
#endif
	}

END_SHADER



BEGIN_VS_SHADER( FULLSCREENQUAD_WRITEA, "" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( COMBINEMODE, SHADER_PARAM_TYPE_BOOL, "", "" )
		SHADER_PARAM( BASETEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
		LoadTexture( BASETEXTURE );
		if ( params[COMBINEMODE]->GetIntValue() )
			LoadTexture( BASETEXTURE2 );
	}

	SHADER_FALLBACK
	{
		return 0;
	}
	
	SHADER_DRAW
	{
		const bool bCombineMode = !!params[COMBINEMODE]->GetIntValue();

		SHADOW_STATE
		{
			pShaderShadow->SetDefaultState();

			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableDepthTest( true );
			pShaderShadow->EnableColorWrites( false );
			pShaderShadow->EnableAlphaWrites( true );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			if ( bCombineMode )
				pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			VERTEXSHADERVERTEXFORMAT( VERTEX_POSITION, 1, 0, 0 );

			pShaderShadow->SetVertexShader( "fbquad_vs20", 0 );
			pShaderShadow->SetPixelShader( "fbquad_ps20", bCombineMode ? 1 : 0 );
		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();

			pShaderAPI->SetVertexShaderIndex();
			pShaderAPI->SetPixelShaderIndex( 0 );

			BindTexture( SHADER_SAMPLER0, BASETEXTURE );
			if ( bCombineMode )
				BindTexture( SHADER_SAMPLER1, BASETEXTURE2 );
		}
#ifdef SHADER_EDITOR_DLL_SWARM
		Draw( true, false );
#else
		Draw();
#endif
	}

END_SHADER