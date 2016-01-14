#ifndef VNODE_SAMPLER_H
#define VNODE_SAMPLER_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

class CNodeSampler : public CBaseNode
{
	DECLARE_CLASS( CNodeSampler, CBaseNode );

public:
	CNodeSampler( CNodeView *p );
	~CNodeSampler();

	virtual int GetNodeType(){ return HLSLNODE_TEXTURE_SAMPLEROBJECT; };
	//virtual bool IsPreviewVisible(){ return false; };
	//virtual const bool IsPreviewEnabled(){ return false; };

	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	virtual void UpdateOutputs();

private:
	char m_szDemoTexturePath[ MAX_PATH ];
	char m_szFallbackTexturePath[ MAX_PATH ];

	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	char szParamName[MAX_PATH];
	bool bIs3DTexture;
	int iTexType;
	int iFallbackType;
};

#endif
