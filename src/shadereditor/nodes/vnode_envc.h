#ifndef VNODE_ENVC_H
#define VNODE_ENVC_H
#ifdef _WIN32
#pragma once
#endif

#include "vbasenode.h"

class CNodeEnvC : public CBaseNode
{
	DECLARE_CLASS( CNodeEnvC, CBaseNode );

public:
	CNodeEnvC( CNodeView *p, float defaultSmartVal );
	~CNodeEnvC();

	enum
	{
		ENVC_FBSCALE_FULL = 0,
		ENVC_FBSCALE_HALF,
		ENVC_FBSCALE_QUARTER,
	};

	void SetEnvCType( int type );
	virtual int GetNodeType(){ return m_iNodeType; };

	virtual int GetAllowedHierachiesAsFlags();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int m_iNodeType;
	int m_iEnvCIndex;

	float m_flDefaultSmartVal;
	float m_flSmartVal;
};

#endif
