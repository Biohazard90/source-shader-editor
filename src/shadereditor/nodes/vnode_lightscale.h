#ifndef VNODE_LIGHTSCALE_H
#define VNODE_LIGHTSCALE_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

class CNodeLightscale : public CBaseNode
{
	DECLARE_CLASS( CNodeLightscale, CBaseNode );

public:

	CNodeLightscale( CNodeView *p );
	~CNodeLightscale();

	virtual int GetNodeType(){ return HLSLNODE_CONSTANT_LIGHTSCALE; };
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	void UpdateNode();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int m_iLightScale;
};

#endif
