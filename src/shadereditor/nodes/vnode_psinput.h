#ifndef CNODEPSINPUT_H
#define CNODEPSINPUT_H

#include <editorCommon.h>

class CNodePSInput : public CBaseNode
{
	DECLARE_CLASS( CNodePSInput, CBaseNode );

public:

	CNodePSInput( CNodeView *p );
	~CNodePSInput();

	virtual bool CanBeInContainer(){ return false; };
	virtual int GetNodeType(){ return HLSLNODE_PS_IN; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsNodeCrucial(){ return true; };
	virtual int PerNodeErrorLevel();

	virtual int GetHierachyType(){return HLSLHIERACHY_PS;};
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };
	virtual int UpdateInputsValid(){ return ERRORLEVEL_NONE; };

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