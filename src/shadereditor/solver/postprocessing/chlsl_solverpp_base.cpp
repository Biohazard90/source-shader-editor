
#include "cbase.h"
#include "editorcommon.h"


CHLSL_Solver_PP_Base::CHLSL_Solver_PP_Base( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx )
{
}

void CHLSL_Solver_PP_Base::Invoke_ExecuteCode( const RunCodeContext &context )
{
	if ( context.IsSceneMode() != IsSceneSolver( context ) )
		return;

	CHLSL_SolverBase::Invoke_ExecuteCode( context );
}

void CHLSL_Solver_PP_Base::BlitRTRect( const RunCodeContext &context )
{
	if ( !context.IsUsingPreview() || !IsInEditMode() )
		return;

	ITexture *pTex = GetPingPongRT(2);

	int x,y,w,t;
	GetCurPuzzleView( x, y, w, t );

	Rect_t dst;
	dst.x = x;
	dst.y = y;
	dst.width = w;
	dst.height = t;
	context.pRenderContext->CopyRenderTargetToTextureEx( pTex, 0, NULL, &dst );

	m_iMapIndex = ::GetCurrentPuzzleIndex();
	OnPuzzleDrawn();
}


void CHLSL_Solver_PP_Start::OnExecuteCode( const RunCodeContext &context )
{
	if ( IsInEditMode() )
		ResetTileInfo();
}