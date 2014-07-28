#ifndef CNODEDOT_H
#define CNODEDOT_H

#include "vBaseNode.h"

class CNodeDot : public CBaseNode
{
	DECLARE_CLASS( CNodeDot, CBaseNode );

public:

	CNodeDot( CNodeView *p );
	~CNodeDot();

	virtual int GetNodeType(){ return HLSLNODE_MATH_DOT; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif