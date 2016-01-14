#ifndef VNODE_STD_VLIGHT_H
#define VNODE_STD_VLIGHT_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

class CNodeStdVLight : public CBaseNode
{
	DECLARE_CLASS( CNodeStdVLight, CBaseNode );

public:

	CNodeStdVLight( CNodeView *p );
	~CNodeStdVLight();

	virtual int GetNodeType(){ return HLSLNODE_STUDIO_VERTEXLIGHTING; };

	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_VS; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };

	virtual void UpdateNode();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	bool m_bStaticLighting;
	bool m_bHalfLambert;
};

#endif
