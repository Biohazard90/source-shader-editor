#ifndef CNODE_SWIZZLE_H
#define CNODE_SWIZZLE_H

#include "vBaseNode.h"

class CNodeSwizzle : public CBaseNode
{
	DECLARE_CLASS( CNodeSwizzle, CBaseNode );

public:

	CNodeSwizzle( CNodeView *p );
	~CNodeSwizzle();

	virtual int GetNodeType(){ return HLSLNODE_MATH_SWIZZLE; };
	virtual int PerNodeErrorLevel();

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	// reading from w or a? must be 4 comps input...
	int GetSwizzleMinComponents();
	// vector targetlength required
	int GetSwizzleTargetComponents();
	//int GetTargetComponentInSlot( int slot );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	TokenChannels_t m_SwizzleInfo;
	char szSwizzle[MAX_PATH];

	void UpdateNode();
};


#endif