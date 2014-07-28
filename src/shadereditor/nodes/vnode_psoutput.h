#ifndef CNODEPSOUTPUT_H
#define CNODEPSOUTPUT_H

#include <editorCommon.h>

class CNodePSOutput : public CBaseNode
{
	DECLARE_CLASS( CNodePSOutput, CBaseNode );

public:

	CNodePSOutput( CNodeView *p );
	~CNodePSOutput();

	virtual bool CanBeInContainer(){ return false; };
	virtual int GetNodeType(){ return HLSLNODE_PS_OUT; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsNodeCrucial(){ return true; };

	virtual int GetHierachyType(){return HLSLHIERACHY_PS;};
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };
	virtual bool ShouldErrorOnUndefined(){ return true; };

	void Setup( SETUP_HLSL_PS_Output info );
	SETUP_HLSL_PS_Output &GetSetup(){ return curSetup; };

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues( KeyValues *pKV );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );


private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	SETUP_HLSL_PS_Output curSetup;
};


#endif