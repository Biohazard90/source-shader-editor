#ifndef CHLSL_SOLVER_PP_RENDERVIEW_H
#define CHLSL_SOLVER_PP_RENDERVIEW_H

#include "editorcommon.h"

class CHLSL_Solver_PP_RenderView : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_RenderView( HNODE nodeidx );
	~CHLSL_Solver_PP_RenderView();
	CHLSL_Solver_PP_RenderView( const CHLSL_Solver_PP_RenderView &o );
	virtual CHLSL_Solver_PP_RenderView *Copy(){
		return new CHLSL_Solver_PP_RenderView( *this ); };

	virtual bool IsSceneSolver( const RunCodeContext &context );

	void Init( EditorRenderViewCommand_Data *pVrCallback, bool bSceneRender );

protected:
	virtual void OnExecuteCode( const RunCodeContext &context );

private:
	EditorRenderViewCommand_Data *m_VrCallback;

	bool m_bAsScene;

};

#endif