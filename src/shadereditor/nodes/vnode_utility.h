#ifndef CNODE_UTILITY_H
#define CNODE_UTILITY_H

#include "vBaseNode.h"

class CNodeUtility_Declare : public CBaseNode
{
	DECLARE_CLASS( CNodeUtility_Declare, CBaseNode );

public:

	CNodeUtility_Declare( CNodeView *p );
	~CNodeUtility_Declare();

	virtual int GetNodeType(){ return HLSLNODE_UTILITY_DECLARE; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


class CNodeUtility_Assign : public CBaseNode
{
	DECLARE_CLASS( CNodeUtility_Assign, CBaseNode );

public:

	CNodeUtility_Assign( CNodeView *p );
	~CNodeUtility_Assign();

	virtual int GetNodeType(){ return HLSLNODE_UTILITY_ASSIGN; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif