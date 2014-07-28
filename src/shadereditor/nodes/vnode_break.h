#ifndef CNODE_BREAK_H
#define CNODE_BREAK_H

#include "vBaseNode.h"

class CNodeBreak : public CBaseNode
{
	DECLARE_CLASS( CNodeBreak, CBaseNode );

public:

	CNodeBreak( CNodeView *p );
	~CNodeBreak();

	virtual int GetNodeType(){ return HLSLNODE_CONTROLFLOW_BREAK; };

	virtual void UpdateNode();

	//virtual void OnUpdateHierachy( CBridge *pBridgeInitiator, CJack *pCaller );
	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };

	virtual void PreSolverUpdated();
	virtual void PostSolverUpdated();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};

class CNodeClip : public CBaseNode
{
	DECLARE_CLASS( CNodeClip, CBaseNode );

public:

	CNodeClip( CNodeView *p );
	~CNodeClip();

	virtual int GetNodeType(){ return HLSLNODE_CONTROLFLOW_CLIP; };

	//virtual void UpdateNode();

	//virtual void OnUpdateHierachy( CBridge *pBridgeInitiator, CJack *pCaller );
	//virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif