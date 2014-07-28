#ifndef CNODE_PP_MAT_H
#define CNODE_PP_MAT_H

#include <editorCommon.h>

class CNodePP_Mat : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_Mat, CNodePP_Base );
public:

	CNodePP_Mat( CNodeView *p );
	~CNodePP_Mat();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_CREATE_MAT; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };
	virtual int UpdateInputsValid();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	char m_szMatPath[MAX_PATH*4];
	char *m_pszInlineVMT;

	bool m_bInline;
};

#endif