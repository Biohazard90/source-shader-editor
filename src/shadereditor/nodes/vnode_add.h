#ifndef VNODE_ADD_H
#define VNODE_ADD_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

class CNodeAdd : public CBaseNode
{
	DECLARE_CLASS( CNodeAdd, CBaseNode );

public:

	CNodeAdd( CNodeView *p );
	~CNodeAdd();

	virtual int GetNodeType(){ return HLSLNODE_MATH_ADD; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};

#endif
