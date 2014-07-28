#ifndef CNODE_PARALLAX_H
#define CNODE_PARALLAX_H

#include "vBaseNode.h"
#include "editorcommon.h"

class CNodeParallax : public CBaseNode
{
	DECLARE_CLASS( CNodeParallax, CBaseNode );

public:

	CNodeParallax( CNodeView *p );
	~CNodeParallax();

	virtual int GetNodeType(){ return HLSLNODE_TEXTURE_PARALLAX; };
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	void UpdateNode();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	//int iSamp_Min;
	//int iSamp_Max;
	//bool bDepthTest;
	ParallaxConfig_t m_PSetup;
};

class CNodeParallax_StdShadow : public CBaseNode
{
	DECLARE_CLASS( CNodeParallax_StdShadow, CBaseNode );
public:

	CNodeParallax_StdShadow( CNodeView *p );
	~CNodeParallax_StdShadow();

	virtual int GetNodeType(){ return HLSLNODE_TEXTURE_PARALLAX_SHADOW; };
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};

#endif