#ifndef CNODE_CALLBACK_H
#define CNODE_CALLBACK_H

#include "vBaseNode.h"

class CNodeCallback : public CBaseNode
{
	DECLARE_CLASS( CNodeCallback, CBaseNode );

public:

	CNodeCallback( CNodeView *p );
	~CNodeCallback();

	virtual int GetNodeType(){ return HLSLNODE_CONSTANT_CALLBACK; };

	void UpdateNode();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	virtual int UpdateInputsValid();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	char *szCallbackName;
	int iNumComponents;
};

#endif