#ifndef CNODE_FINAL_H
#define CNODE_FINAL_H

#include "vBaseNode.h"

class CNodeFinal : public CBaseNode
{
	DECLARE_CLASS( CNodeFinal, CBaseNode );

public:

	CNodeFinal( CNodeView *p );
	~CNodeFinal();

	virtual int GetNodeType(){ return HLSLNODE_UTILITY_FINAL; };

	virtual void UpdateNode();

	virtual void SetState( int t, bool wd );

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int i_tonemaptype;
	bool b_writedepth;
};


#endif