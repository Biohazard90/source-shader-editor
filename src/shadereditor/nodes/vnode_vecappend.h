#ifndef CNODE_APPEND_H
#define CNODE_APPEND_H

#include "vBaseNode.h"

class CNodeAppend : public CBaseNode
{
	DECLARE_CLASS( CNodeAppend, CBaseNode );

public:

	CNodeAppend( CNodeView *p );
	~CNodeAppend();

	virtual int GetNodeType(){ return HLSLNODE_MATH_APPEND; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif