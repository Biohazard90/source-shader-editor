#ifndef VNODE_DOT_H
#define VNODE_DOT_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

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
