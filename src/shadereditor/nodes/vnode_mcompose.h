#ifndef CNODE_MCOMPOSE_H
#define CNODE_MCOMPOSE_H

#include "vBaseNode.h"

class CNodeMCompose : public CBaseNode
{
	DECLARE_CLASS( CNodeMCompose, CBaseNode );

public:

	CNodeMCompose( CNodeView *p );
	~CNodeMCompose();

	virtual int GetNodeType(){ return HLSLNODE_MATRIX_COMPOSE; };

	void SetTargetMatrix( int flag, bool docolumns );
	void UpdateNode();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int iTargetMatrix;
	bool bColumns;
};


#endif