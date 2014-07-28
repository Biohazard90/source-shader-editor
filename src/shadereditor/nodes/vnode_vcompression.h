#ifndef CNODE_VCOMPRESSION_H
#define CNODE_VCOMPRESSION_H

#include "vBaseNode.h"

class CNodeVCompression : public CBaseNode
{
	DECLARE_CLASS( CNodeVCompression, CBaseNode );

public:

	enum
	{
		COMPRESSION_MODE_NORMAL = 0,
		COMPRESSION_MODE_NORMAL_TANGENT,

		COMPRESSION_MODE_COUNT,
	};

	CNodeVCompression( CNodeView *p );
	~CNodeVCompression();

	virtual int GetNodeType(){ return HLSLNODE_STUDIO_VCOMPRESSION; };

	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_VS; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };

	//virtual int UpdateInputsValid();
	//virtual void UpdateOutputs();

	virtual void UpdateNode();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int m_iCompressionMode;
};


#endif