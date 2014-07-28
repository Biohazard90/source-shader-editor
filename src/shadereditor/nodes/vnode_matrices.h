#ifndef CNODEMATRICES_H
#define CNODEMATRICES_H

#include "vBaseNode.h"

class CNodeMatrix_MVP : public CBaseNode
{
	DECLARE_CLASS( CNodeMatrix_MVP, CBaseNode );
public:
	CNodeMatrix_MVP( CNodeView *p );
	virtual int GetNodeType(){ return HLSLNODE_MATRIX_MODELVIEWPROJ; };
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_VS; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };
private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);
};


class CNodeMatrix_VP : public CBaseNode
{
	DECLARE_CLASS( CNodeMatrix_VP, CBaseNode );
public:
	CNodeMatrix_VP( CNodeView *p );
	virtual int GetNodeType(){ return HLSLNODE_MATRIX_VIEWPROJ; };
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_VS; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };
private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);
};


class CNodeMatrix_M : public CBaseNode
{
	DECLARE_CLASS( CNodeMatrix_M, CBaseNode );
public:
	CNodeMatrix_M( CNodeView *p );
	virtual int GetNodeType(){ return HLSLNODE_MATRIX_MODEL; };
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_VS; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };
private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);
};



class CNodeMatrix_VM : public CBaseNode
{
	DECLARE_CLASS( CNodeMatrix_VM, CBaseNode );
public:
	CNodeMatrix_VM( CNodeView *p );
	virtual int GetNodeType(){ return HLSLNODE_MATRIX_VIEWMODEL; };
	virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_VS; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };
private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);
};




class CNodeMatrix_FVP : public CBaseNode
{
	DECLARE_CLASS( CNodeMatrix_FVP, CBaseNode );
public:
	CNodeMatrix_FVP( CNodeView *p );
	virtual int GetNodeType(){ return HLSLNODE_MATRIX_FLASHLIGHT; };
	//virtual int GetAllowedHierachiesAsFlags(){ return HLSLHIERACHY_PS; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };
private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);
};


class CNodeMatrix_Custom : public CBaseNode
{
	DECLARE_CLASS( CNodeMatrix_Custom, CBaseNode );
public:
	CNodeMatrix_Custom( CNodeView *p );
	virtual int GetNodeType(){ return HLSLNODE_MATRIX_CUSTOM; };
	virtual bool IsPreviewVisible(){ return false; };
	virtual const bool IsPreviewEnabled(){ return false; };

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );
	void UpdateNode();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int m_iCustomID;
};

#endif