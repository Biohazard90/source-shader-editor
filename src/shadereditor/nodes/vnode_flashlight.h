#ifndef CNODE_FLASHLIGHT_H
#define CNODE_FLASHLIGHT_H

#include "vBaseNode.h"

class CNodeFlashlight : public CBaseNode
{
	DECLARE_CLASS( CNodeFlashlight, CBaseNode );

public:

	CNodeFlashlight( CNodeView *p );
	~CNodeFlashlight();

	virtual int GetNodeType(){ return HLSLNODE_UTILITY_FLASHLIGHT; };
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	void UpdateNode();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	bool m_bSpecular;
};



class CNodeFlashlight_Position : public CBaseNode
{
	DECLARE_CLASS( CNodeFlashlight_Position, CBaseNode );

public:

	CNodeFlashlight_Position( CNodeView *p );
	~CNodeFlashlight_Position();

	virtual int GetNodeType(){ return HLSLNODE_CONSTANT_FLASHLIGHTPOS; };
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);
};


#endif