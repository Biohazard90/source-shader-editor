#ifndef CNODE_STD_PLIGHT_H
#define CNODE_STD_PLIGHT_H

#include "vBaseNode.h"

class CNodeStdPLight : public CBaseNode
{
	DECLARE_CLASS( CNodeStdPLight, CBaseNode );

public:

	CNodeStdPLight( CNodeView *p );
	~CNodeStdPLight();

	virtual int GetNodeType(){ return HLSLNODE_STUDIO_PIXELSHADER_LIGHTING; };

	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };

	virtual void UpdateNode();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	bool m_bStaticLighting;
	bool m_bAOTerm;
	bool m_bHalfLambert;
};



class CNodeStdPSpecLight : public CBaseNode
{
	DECLARE_CLASS( CNodeStdPSpecLight, CBaseNode );

public:

	CNodeStdPSpecLight( CNodeView *p );
	~CNodeStdPSpecLight();

	virtual int GetNodeType(){ return HLSLNODE_STUDIO_PIXELSHADER_SPECLIGHTING; };

	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };

	virtual void UpdateNode();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	bool m_bAOTerm;
};


#endif