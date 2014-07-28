#ifndef CNODE_LERP_H
#define CNODE_LERP_H

#include "vBaseNode.h"

class CNodeLerp : public CBaseNode
{
	DECLARE_CLASS( CNodeLerp, CBaseNode );

public:

	CNodeLerp( CNodeView *p );
	~CNodeLerp();

	virtual int GetNodeType(){ return HLSLNODE_MATH_LERP; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif