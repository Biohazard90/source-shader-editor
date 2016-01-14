#ifndef VNODE_CROSS_H
#define VNODE_CROSS_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

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
