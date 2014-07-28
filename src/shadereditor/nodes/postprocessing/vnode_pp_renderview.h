#ifndef CNODE_PP_RENDERVIEW_H
#define CNODE_PP_RENDERVIEW_H

#include <editorCommon.h>

class CNodePP_RenderView : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_RenderView, CNodePP_Base );
public:

	CNodePP_RenderView( CNodeView *p );
	~CNodePP_RenderView();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_RENDER_VIEW; };
	virtual int UpdateInputsValid();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	virtual const bool IsSceneNode(){ return m_bSceneDraw; };

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	EditorRenderViewCommand_Data *m_ViewRenderData;
	bool	m_bSceneDraw;

};

#endif