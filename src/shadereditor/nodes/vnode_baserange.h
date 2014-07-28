#ifndef CNODE_BASERANGE_H
#define CNODE_BASERANGE_H

#include "vBaseNode.h"

class CNodeBaseRange : public CBaseNode
{
	DECLARE_CLASS( CNodeBaseRange, CBaseNode );

public:

	CNodeBaseRange( const char *name, CNodeView *p );
	~CNodeBaseRange();

	virtual int GetNodeType(){ return HLSLNODE_INVALID; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

protected:
	virtual CHLSL_SolverBase *AllocRangeSolver(){ Assert(0); return NULL; };

};


class CNodeSmoothstep : public CNodeBaseRange
{
	DECLARE_CLASS( CNodeSmoothstep, CNodeBaseRange );
public:
	CNodeSmoothstep( CNodeView *p );
	virtual int GetNodeType(){ return HLSLNODE_MATH_SMOOTHSTEP; };
protected:
	virtual CHLSL_SolverBase *AllocRangeSolver();
};
class CNodeClamp : public CNodeBaseRange
{
	DECLARE_CLASS( CNodeClamp, CNodeBaseRange );
public:
	CNodeClamp( CNodeView *p );
	virtual int GetNodeType(){ return HLSLNODE_MATH_CLAMP; };
protected:
	virtual CHLSL_SolverBase *AllocRangeSolver();
};

#endif