#ifndef CNODE_PP_OPERATIONS_H
#define CNODE_PP_OPERATIONS_H

#include <editorCommon.h>

class CNodePP_ClearBuffers : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_ClearBuffers, CNodePP_Base );
public:

	CNodePP_ClearBuffers( CNodeView *p );
	~CNodePP_ClearBuffers();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_CLEAR_BUFFERS; };
	virtual int UpdateInputsValid();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	TokenChannels_t m_iChannelInfo;
	char m_szConstantString[MAX_PATH];

	bool m_bClearColor;
	bool m_bClearDepth;
};


class CNodePP_VP_Push : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_VP_Push, CNodePP_Base );
public:

	CNodePP_VP_Push( CNodeView *p );
	~CNodePP_VP_Push();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_PUSH_VP; };
	virtual int UpdateInputsValid();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


class CNodePP_VP_Pop : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_VP_Pop, CNodePP_Base );
public:

	CNodePP_VP_Pop( CNodeView *p );
	~CNodePP_VP_Pop();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_POP_VP; };
	virtual int UpdateInputsValid();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


class CNodePP_VP_SetRT : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_VP_SetRT, CNodePP_Base );
public:

	CNodePP_VP_SetRT( CNodeView *p );
	~CNodePP_VP_SetRT();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_SET_RT; };
	virtual int UpdateInputsValid();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


class CNodePP_CopyRT : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_CopyRT, CNodePP_Base );
public:

	CNodePP_CopyRT( CNodeView *p );
	~CNodePP_CopyRT();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_COPY_RT; };
	virtual int UpdateInputsValid();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


class CNodePP_UpdateFB : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_UpdateFB, CNodePP_Base );
public:

	CNodePP_UpdateFB( CNodeView *p );
	~CNodePP_UpdateFB();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_UPDATE_FB; };

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif