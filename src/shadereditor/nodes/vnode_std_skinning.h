#ifndef CNODE_STD_SKINNING_H
#define CNODE_STD_SKINNING_H

#include "vBaseNode.h"


enum
{
	STD_SKINNING_MODE_POS = 0,
	STD_SKINNING_MODE_POS_NORMAL,
	STD_SKINNING_MODE_POS_NORMAL_TANGENT,
};

class CNodeStdSkinning : public CBaseNode
{
	DECLARE_CLASS( CNodeStdSkinning, CBaseNode );

public:

	CNodeStdSkinning( CNodeView *p );
	~CNodeStdSkinning();

	virtual int GetNodeType(){ return HLSLNODE_STUDIO_SKINNING; };

	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_VS; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };

	virtual void SetState( int m );

	virtual void UpdateNode();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int m_iSkinMode;
};


class CNodeStdMorph : public CBaseNode
{
	DECLARE_CLASS( CNodeStdMorph, CBaseNode );

public:

	CNodeStdMorph( CNodeView *p );
	~CNodeStdMorph();

	virtual int GetNodeType(){ return HLSLNODE_STUDIO_MORPH; };

	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_VS; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };

	virtual void SetState( int m );

	virtual void UpdateNode();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int m_iSkinMode;
};


#endif