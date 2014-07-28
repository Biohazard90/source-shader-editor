#ifndef CNODE_CONSTANT_H
#define CNODE_CONSTANT_H

#include "vBaseNode.h"

class CNodeConstant : public CBaseNode
{
	DECLARE_CLASS( CNodeConstant, CBaseNode );

public:

	CNodeConstant( CNodeView *p );
	~CNodeConstant();

	virtual int GetNodeType(){ return HLSLNODE_CONSTANT_LOCAL; };

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	bool VguiDraw( bool bShadow = false );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	TokenChannels_t m_iChannelInfo;
	char m_szConstantString[MAX_PATH];

	void UpdateNode();
};


class CNodeRandom : public CBaseNode
{
	DECLARE_CLASS( CNodeRandom, CBaseNode );

public:
	CNodeRandom( CNodeView *p );
	~CNodeRandom();

	virtual int GetNodeType(){ return HLSLNODE_CONSTANT_RANDOM; };

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int m_iNumComponents;
	float m_flValue_Min;
	float m_flValue_Max;
	void UpdateNode();
};


class CNodeBumpBasis : public CBaseNode
{
	DECLARE_CLASS( CNodeBumpBasis, CBaseNode );

public:
	CNodeBumpBasis( CNodeView *p );
	~CNodeBumpBasis();

	virtual int GetNodeType(){ return HLSLNODE_CONSTANT_BUMPBASIS; };

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif