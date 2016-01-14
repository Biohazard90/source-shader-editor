#ifndef VNODE_FOG_H
#define VNODE_FOG_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

class CNodeFog : public CBaseNode
{
	DECLARE_CLASS( CNodeFog, CBaseNode );

public:

	CNodeFog( CNodeView *p );
	~CNodeFog();

	virtual int GetNodeType(){ return HLSLNODE_UTILITY_FOG; };

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};

#endif
