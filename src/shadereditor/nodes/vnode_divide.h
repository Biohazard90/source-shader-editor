#ifndef CNODE_DIVIDE_H
#define CNODE_DIVIDE_H

#include "vBaseNode.h"

class CNodeDivide : public CBaseNode
{
	DECLARE_CLASS( CNodeDivide, CBaseNode );

public:

	CNodeDivide( CNodeView *p );
	~CNodeDivide();

	virtual int GetNodeType(){ return HLSLNODE_MATH_DIVIDE; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif