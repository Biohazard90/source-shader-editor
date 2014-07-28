
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_PP_RenderView::CHLSL_Solver_PP_RenderView( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx )
{
	m_VrCallback = NULL;
	m_bAsScene = true;
}
CHLSL_Solver_PP_RenderView::~CHLSL_Solver_PP_RenderView()
{
	delete m_VrCallback;
}
CHLSL_Solver_PP_RenderView::CHLSL_Solver_PP_RenderView( const CHLSL_Solver_PP_RenderView &o ) : CHLSL_Solver_PP_Base( o )
{
	m_VrCallback = new EditorRenderViewCommand_Data( *o.m_VrCallback );
	m_bAsScene = o.m_bAsScene;
}

bool CHLSL_Solver_PP_RenderView::IsSceneSolver( const RunCodeContext &context )
{
	return m_bAsScene;
}

void CHLSL_Solver_PP_RenderView::Init( EditorRenderViewCommand_Data *pVrCallback, bool bSceneRender )
{
	m_VrCallback = new EditorRenderViewCommand_Data( *pVrCallback );

	m_VrCallback->ValidateMemory();

	m_bAsScene = bSceneRender;
}

void CHLSL_Solver_PP_RenderView::OnExecuteCode( const RunCodeContext &context )
{
	bool bMatOverride = GetNumSourceVars() > 1;

	context.pRenderContext->PushRenderTargetAndViewport( GetSourceVar(0)->GetTexture() );

	if ( bMatOverride )
		modelrender->ForcedMaterialOverride( GetSourceVar(1)->GetMaterial() );

	m_VrCallback->CallFunction();

	if ( bMatOverride )
		modelrender->ForcedMaterialOverride( NULL );

	BlitRTRect( context );

	context.pRenderContext->PopRenderTargetAndViewport();
}