#ifndef CNODE_PP_RT_H
#define CNODE_PP_RT_H

#include <editorCommon.h>

class CNodePP_RT : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_RT, CNodePP_Base );
public:

	CNodePP_RT( CNodeView *p );
	~CNodePP_RT();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_CREATE_RT; };
	virtual int UpdateInputsValid();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	char m_szRTName[RT_NAME_MAX];
};

#endif