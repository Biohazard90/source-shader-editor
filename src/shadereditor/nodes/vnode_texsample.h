#ifndef CNODETEXTURESAMPLE_H
#define CNODETEXTURESAMPLE_H

#include "vBaseNode.h"

class CNodeTexSample : public CBaseNode
{
	DECLARE_CLASS( CNodeTexSample, CBaseNode );

public:
	CNodeTexSample( CNodeView *p );
	~CNodeTexSample();

	virtual int GetNodeType(){ return HLSLNODE_TEXTURE_SAMPLER; };
#ifndef SHADER_EDITOR_DLL_SWARM
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };
#endif

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	char m_szDemoTexturePath[ MAX_PATH ];
	char m_szFallbackTexturePath[ MAX_PATH ];

	void UpdateInput();
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	char szParamName[MAX_PATH];
	bool bSRGBRead;
	bool bIs3DTexture;
	int iTexType;
	int iFallbackType;
	int iLookupOverride;
};


#endif