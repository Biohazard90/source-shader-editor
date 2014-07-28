
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_PP_ClearBuffers::Init( const TokenChannels_t &values, bool bColor, bool bDepth )
{
	m_iChannelInfo = values;

	bClearColor = bColor;
	bClearDepth = bDepth;
}

void CHLSL_Solver_PP_ClearBuffers::OnExecuteCode( const RunCodeContext &context )
{
	CHLSL_Var *pVar = ( GetNumSourceVars() > 0 ) ? GetSourceVar(0) : NULL;
	const bool bDoPush = pVar != NULL && pVar->GetTexture() != NULL;

	if ( bDoPush )
		context.pRenderContext->PushRenderTargetAndViewport( pVar->GetTexture() );

	unsigned char colors[4] = { 255, 255, 255, 255 };
	for ( int i = 0; i < m_iChannelInfo.iActiveChannels; i++ )
	{
		colors[i] = (unsigned char)( m_iChannelInfo.flValue[i] * 255.0f );
	}

	context.pRenderContext->ClearColor4ub( colors[0], colors[1], colors[2], colors[3] );
	context.pRenderContext->ClearBuffers( bClearColor, bClearDepth );

	BlitRTRect( context );

	if ( bDoPush )
		context.pRenderContext->PopRenderTargetAndViewport();
}



void CHLSL_Solver_PP_VP_Push::OnExecuteCode( const RunCodeContext &context )
{
	CHLSL_Var *pVar = ( GetNumSourceVars() > 0 ) ? GetSourceVar(0) : NULL;

	ITexture *pRT = pVar ? pVar->GetTexture() : NULL;

	context.pRenderContext->PushRenderTargetAndViewport( pRT );

	BlitRTRect( context );
}



void CHLSL_Solver_PP_VP_Pop::OnExecuteCode( const RunCodeContext &context )
{
	context.pRenderContext->PopRenderTargetAndViewport();

	BlitRTRect( context );
}



void CHLSL_Solver_PP_VP_SetRT::OnExecuteCode( const RunCodeContext &context )
{
	CHLSL_Var *pVar = ( GetNumSourceVars() > 0 ) ? GetSourceVar(0) : NULL;
	ITexture *pRT = pVar ? pVar->GetTexture() : NULL;

	int x,y;
	materials->GetBackBufferDimensions( x, y );

	if ( pRT )
	{
		x = pRT->GetActualWidth();
		y = pRT->GetActualHeight();
	}

	context.pRenderContext->SetRenderTarget( pRT );
	context.pRenderContext->Viewport( 0, 0, x, y );

	BlitRTRect( context );
}



void CHLSL_Solver_PP_CopyRT::OnExecuteCode( const RunCodeContext &context )
{
	CHLSL_Var *pVar_tg = GetSourceVar(0);
	CHLSL_Var *pVar_src = ( GetNumSourceVars() > 1 ) ? GetSourceVar(1) : NULL;
	ITexture *pRT_tg = pVar_tg->GetTexture();
	ITexture *pRT_src = pVar_src ? pVar_src->GetTexture() : NULL;

	const bool bDoPush = pRT_src != NULL;

	if ( bDoPush )
		context.pRenderContext->PushRenderTargetAndViewport( pRT_src );

	context.pRenderContext->CopyRenderTargetToTexture( pRT_tg );

	BlitRTRect( context );

	if ( bDoPush )
		context.pRenderContext->PopRenderTargetAndViewport();
}

void CHLSL_Solver_PP_UpdateFB::OnExecuteCode( const RunCodeContext &context )
{
	GeneralFramebufferUpdate( context.pRenderContext );

	if ( context.IsUsingPreview() )
	{
		context.pRenderContext->PushRenderTargetAndViewport( GetFBTex() );
		BlitRTRect( context );
		context.pRenderContext->PopRenderTargetAndViewport();
	}
}
