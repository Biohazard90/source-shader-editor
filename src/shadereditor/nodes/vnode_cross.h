#ifndef CNODE_CROSS_H
#define CNODE_CROSS_H

#include "vBaseNode.h"

class CNodeCross : public CBaseNode
{
	DECLARE_CLASS( CNodeCross, CBaseNode );

public:

	CNodeCross( CNodeView *p );
	~CNodeCross();

	virtual int GetNodeType(){ return HLSLNODE_VECTOR_CROSS; };

	virtual int UpdateInputsValid();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif