#ifndef VNODE_TEXTRANSFORM_H
#define VNODE_TEXTRANSFORM_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

class CNodeTexTransform : public CBaseNode
{
	DECLARE_CLASS( CNodeTexTransform, CBaseNode );

public:

	CNodeTexTransform( CNodeView *p );
	~CNodeTexTransform();

	virtual int GetNodeType(){ return HLSLNODE_TEXTURE_TRANSFORM; };

	void UpdateNode();
	virtual int UpdateInputsValid();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	bool bEnable_Center;
	bool bEnable_Rot;
	bool bEnable_Scale;
	bool bEnable_Trans;
};

#endif
