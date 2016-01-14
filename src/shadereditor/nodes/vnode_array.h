#ifndef VNODE_ARRAY_H
#define VNODE_ARRAY_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

class CNodeArray : public CBaseNode
{
	DECLARE_CLASS( CNodeArray, CBaseNode );

public:
	CNodeArray( CNodeView *p );
	~CNodeArray();

	void UpdateNode();

	virtual int GetNodeType(){ return HLSLNODE_CONSTANT_ARRAY; };

	//virtual int UpdateInputsValid();
	//virtual void UpdateOutputs();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	//CNodeArray( const CNodeArray &other ){};
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int m_iSize_X;
	int m_iSize_Y;

	int m_iDataTypeFlag;

	Vector4D *m_vecValues;
};

#endif
