
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_PP_DrawMat::CHLSL_Solver_PP_DrawMat( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx )
{
	m_bPushTarget = false;
	m_iSizingMode_tg = CNodePP_DrawMat::VPSIZINGMODE_FULL_FB;
	Q_memset( &m_offsets, 0, sizeof( vp_offsets_t ) );
}

void CHLSL_Solver_PP_DrawMat::Init( int sizing_tg, int sizing_src, bool bPushTarget, vp_offsets_t offsets )
{
	m_iSizingMode_tg = sizing_tg;
	m_iSizingMode_src = sizing_src;
	m_bPushTarget = bPushTarget;
	m_offsets = offsets;
}

void CHLSL_Solver_PP_DrawMat::OnExecuteCode( const RunCodeContext &context )
{
	int backbuffer_sx, backbuffer_sy;
	materials->GetBackBufferDimensions( backbuffer_sx, backbuffer_sy );

	IMaterial *pMat = GetSourceVar(0)->GetMaterial();

	int iSrcVarIndex = 1;
	int tg_sx, tg_sy;
	int src_sx, src_sy;

	ITexture *pRT_Target = NULL;

	switch ( m_iSizingMode_tg )
	{
	default:
	case CNodePP_DrawMat::VPSIZINGMODE_FULL_FB:
		tg_sx = backbuffer_sx;
		tg_sy = backbuffer_sy;
		break;
	case CNodePP_DrawMat::VPSIZINGMODE_USE_RT:
		{
			pRT_Target = GetSourceVar(iSrcVarIndex)->GetTexture();
			tg_sx = pRT_Target->GetActualWidth();
			tg_sy = pRT_Target->GetActualHeight();
			iSrcVarIndex++;
		}
		break;
	case CNodePP_DrawMat::VPSIZINGMODE_HALF_FB:
		tg_sx = backbuffer_sx / 2;
		tg_sy = backbuffer_sy / 2;
		break;
	case CNodePP_DrawMat::VPSIZINGMODE_QUARTER_FB:
		tg_sx = backbuffer_sx / 4;
		tg_sy = backbuffer_sy / 4;
		break;
	}

	switch ( m_iSizingMode_src )
	{
	default:
	case CNodePP_DrawMat::VPSIZINGMODE_FULL_FB:
		src_sx = backbuffer_sx;
		src_sy = backbuffer_sy;
		break;
	case CNodePP_DrawMat::VPSIZINGMODE_USE_RT:
		{
			ITexture *pRT_Source = GetSourceVar(iSrcVarIndex)->GetTexture();
			src_sx = pRT_Source->GetActualWidth();
			src_sy = pRT_Source->GetActualHeight();
		}
		break;
	case CNodePP_DrawMat::VPSIZINGMODE_HALF_FB:
		src_sx = backbuffer_sx / 2;
		src_sy = backbuffer_sy / 2;
		break;
	case CNodePP_DrawMat::VPSIZINGMODE_QUARTER_FB:
		src_sx = backbuffer_sx / 4;
		src_sy = backbuffer_sy / 4;
		break;
	}

	const bool bDoPush = m_bPushTarget && pRT_Target != NULL;

	if ( bDoPush )
		context.pRenderContext->PushRenderTargetAndViewport( pRT_Target );

	// 1	:: 1600/1600	1
	// 1.5	:: 1600/800		2
	// 2	:: 1600/400		4

	float src_uv_x = src_sx / (float)tg_sx;
	src_uv_x = -1.0f/12.0f * src_uv_x * src_uv_x + 3.0f/4.0f * src_uv_x + 1.0f/3.0f;
	float src_uv_y = src_sy / (float)tg_sy;
	src_uv_y = -1.0f/12.0f * src_uv_y * src_uv_y + 3.0f/4.0f * src_uv_y + 1.0f/3.0f;

	//src_uv_x += m_vecSrcUVCompensate.x;
	//src_uv_y += m_vecSrcUVCompensate.y;

	int fix_x = 0;
	int fix_y = 0;

	if ( src_sx == backbuffer_sx / 2 &&
		src_sx % 2 && tg_sx % 2 )
		fix_x--;
	if ( src_sy == backbuffer_sy / 2 &&
		src_sy % 2 && tg_sy % 2 )
		fix_y--;

	context.pRenderContext->MatrixMode( MATERIAL_VIEW );
	context.pRenderContext->PushMatrix();
	context.pRenderContext->LoadIdentity();
	context.pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	context.pRenderContext->PushMatrix();
	context.pRenderContext->LoadIdentity();
	context.pRenderContext->MatrixMode( MATERIAL_MODEL );
	context.pRenderContext->PushMatrix();
	context.pRenderContext->LoadIdentity();

	context.pRenderContext->DrawScreenSpaceRectangle( pMat,
		m_offsets.dst_x, m_offsets.dst_y, // dest x y
		tg_sx + m_offsets.dst_w, tg_sy + m_offsets.dst_h, // dest w h
		m_offsets.src_x0, m_offsets.src_y0, // src x0 y0
		src_sx - src_uv_x + m_offsets.src_x1, src_sy - src_uv_y + m_offsets.src_y1, // src x1 y1
		src_sx + fix_x + m_offsets.src_w, src_sy + fix_y + m_offsets.src_h ); // src w h

	BlitRTRect( context );

	context.pRenderContext->MatrixMode( MATERIAL_MODEL );
	context.pRenderContext->PopMatrix();
	context.pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	context.pRenderContext->PopMatrix();
	context.pRenderContext->MatrixMode( MATERIAL_VIEW );
	context.pRenderContext->PopMatrix();

	if ( bDoPush )
		context.pRenderContext->PopRenderTargetAndViewport();
}
