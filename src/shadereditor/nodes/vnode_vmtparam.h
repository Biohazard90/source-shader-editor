#ifndef CNODE_VMTPARAM_H
#define CNODE_VMTPARAM_H

#include "vBaseNode.h"

class CNodeVmtParam_Mutable : public CBaseNode
{
	DECLARE_CLASS( CNodeVmtParam_Mutable, CBaseNode );

public:
	CNodeVmtParam_Mutable( CNodeView *p );
	~CNodeVmtParam_Mutable(){};

	virtual int GetNodeType(){ return HLSLNODE_CONSTANT_VP_MUTABLE; };

	void UpdateNode();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int m_iParamIndex;
	int m_iNumComponents;

	char m_szConstantString[MAX_PATH];
	TokenChannels_t m_iChannelInfo;
};



class CNodeVmtParam_Static : public CBaseNode
{
	DECLARE_CLASS( CNodeVmtParam_Static, CBaseNode );

public:
	CNodeVmtParam_Static( CNodeView *p );
	~CNodeVmtParam_Static();

	virtual int GetNodeType(){ return HLSLNODE_CONSTANT_VP_STATIC; };

	void UpdateNode();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	char *m_szName;
	int m_iNumComponents;

	char m_szConstantString[MAX_PATH];
	TokenChannels_t m_iChannelInfo;
};


#endif