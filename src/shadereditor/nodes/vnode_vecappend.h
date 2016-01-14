#ifndef VNODE_VECAPPEND_H
#define VNODE_VECAPPEND_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

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
