#ifndef CNODE_VECTORREF_H
#define CNODE_VECTORREF_H

#include "vBaseNode.h"

class CNodeVectorReflect : public CBaseNode
{
	DECLARE_CLASS( CNodeVectorReflect, CBaseNode );

public:
	CNodeVectorReflect( CNodeView *p );
	~CNodeVectorReflect();
	virtual int GetNodeType(){ return HLSLNODE_VECTOR_REFLECT; };
	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};

class CNodeVectorRefract : public CBaseNode
{
	DECLARE_CLASS( CNodeVectorRefract, CBaseNode );

public:
	CNodeVectorRefract( CNodeView *p );
	~CNodeVectorRefract();
	virtual int GetNodeType(){ return HLSLNODE_VECTOR_REFRACT; };
	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif