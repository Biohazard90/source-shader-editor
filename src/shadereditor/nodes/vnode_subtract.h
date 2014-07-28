#ifndef CNODE_SUBTRACT_H
#define CNODE_SUBTRACT_H

#include "vBaseNode.h"

class CNodeSubtract : public CBaseNode
{
	DECLARE_CLASS( CNodeSubtract, CBaseNode );

public:

	CNodeSubtract( CNodeView *p );
	~CNodeSubtract();

	virtual int GetNodeType(){ return HLSLNODE_MATH_SUBTRACT; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};



class CNodeInvert : public CBaseNode
{
	DECLARE_CLASS( CNodeInvert, CBaseNode );

public:

	CNodeInvert( CNodeView *p );
	~CNodeInvert();

	virtual int GetNodeType(){ return HLSLNODE_MATH_INVERT; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif