#ifndef VNODE_MULTIPLY_H
#define VNODE_MULTIPLY_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

class CNodeMultiply : public CBaseNode
{
	DECLARE_CLASS( CNodeMultiply, CBaseNode );

public:

	CNodeMultiply( CNodeView *p );
	~CNodeMultiply();

	virtual int GetNodeType(){ return HLSLNODE_MATH_MULTIPLY; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	bool m_bMatrixRotation;

};

#endif
