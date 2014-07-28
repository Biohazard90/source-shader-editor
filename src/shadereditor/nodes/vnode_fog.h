#ifndef CNODE_FOG_H
#define CNODE_FOG_H

#include "vBaseNode.h"

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