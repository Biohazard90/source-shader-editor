#ifndef CNODE_PP_BASE_H
#define CNODE_PP_BASE_H

#include <editorCommon.h>

class CNodePP_Base : public CBaseNode
{
	DECLARE_CLASS( CNodePP_Base, CBaseNode );
public:

	CNodePP_Base( const char *opName, CNodeView *p );
	~CNodePP_Base();

	virtual const int GetAllowedFlowgraphTypes();

	virtual bool CanBeInContainer(){ return false; };
	//virtual bool IsPreviewVisible(){ return true; };

	virtual int GetHierachyType(){return HLSLHIERACHY_POST_PROCESS;};
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_POST_PROCESS; };

	CNodePP_Base *GetNextPPNode( int dir = 1 );
	virtual const bool IsSceneNode(){ return false; };

};



class CNodePP_Input : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_Input, CNodePP_Base );
public:

	CNodePP_Input( CNodeView *p );
	~CNodePP_Input();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_INPUT; };
	virtual const bool IsNodeCrucial(){ return true; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };

	virtual int UpdateInputsValid(){ return ERRORLEVEL_NONE; };

	//virtual KeyValues *AllocateKeyValues( int NodeIndex );
	//virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


class CNodePP_Output : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_Output, CNodePP_Base );
public:

	CNodePP_Output( CNodeView *p );
	~CNodePP_Output();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_OUTPUT; };
	virtual const bool IsNodeCrucial(){ return true; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };

	virtual int UpdateInputsValid();

	//virtual KeyValues *AllocateKeyValues( int NodeIndex );
	//virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

};


#endif