#ifndef CNODEVSOUTPUT_H
#define CNODEVSOUTPUT_H

#include <editorCommon.h>

class CNodeVSOutput : public CBaseNode
{
	DECLARE_CLASS( CNodeVSOutput, CBaseNode );

public:

	CNodeVSOutput( CNodeView *p );
	~CNodeVSOutput();

	virtual bool CanBeInContainer(){ return false; };
	virtual int GetNodeType(){ return HLSLNODE_VS_OUT; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsNodeCrucial(){ return true; };

	virtual int GetHierachyType(){return HLSLHIERACHY_VS;};
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_VS; };
	virtual bool ShouldErrorOnUndefined(){ return true; };

	void Setup( SETUP_HLSL_VS_Output_PS_Input info );
	SETUP_HLSL_VS_Output_PS_Input &GetSetup(){ return curSetup; };

	void LoadNames();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues( KeyValues *pKV );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	SETUP_HLSL_VS_Output_PS_Input curSetup;
	SETUP_VSOUTPSIN_CustomNames curNames;
};


#endif