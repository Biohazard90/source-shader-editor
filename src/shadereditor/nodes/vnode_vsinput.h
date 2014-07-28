#ifndef CNODEVSINPUT_H
#define CNODEVSINPUT_H

#include <editorCommon.h>

class CNodeVSInput : public CBaseNode
{
	DECLARE_CLASS( CNodeVSInput, CBaseNode );

public:

	CNodeVSInput( CNodeView *p );
	~CNodeVSInput();

	virtual bool CanBeInContainer(){ return false; };
	virtual int GetNodeType(){ return HLSLNODE_VS_IN; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsNodeCrucial(){ return true; };

	virtual int GetHierachyType(){return HLSLHIERACHY_VS;};
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_VS; };
	virtual int UpdateInputsValid(){ return ERRORLEVEL_NONE; };

	void Setup( SETUP_HLSL_VS_Input info );
	SETUP_HLSL_VS_Input &GetSetup(){ return curSetup; };

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	SETUP_HLSL_VS_Input curSetup;
};


#endif