#ifndef CBASENODE_H
#define CBASENODE_H

#include <vgui/VGUI.h>
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include "ienginevgui.h"
#include "vgui_controls/Controls.h"

class CJack;
class CBaseNode;
class CBaseContainerNode;
class CNodeView;

#include "hlsl_solver.h"

#define NODEDEFSIZE_SMALL 50.0f

#define NODE_DRAW_TITLE_Y 15
#define NODE_DRAW_TITLE_SPACE 3
#define PREVIEWINSET 3.5f

#define NODE_DRAW_COLOR_TITLE Color( 60, 60, 70, 255 )
#define NODE_DRAW_COLOR_BOX Color( 110, 110, 110, 255 )
#define NODE_DRAW_COLOR_BOX_COMMENT Color( 18, 18, 18, 255 )
#define NODE_DRAW_COLOR_BORDER Color( 10, 10, 10, 255 )
#define NODE_DRAW_COLOR_TEXT_OP Color( 150, 150, 100, 255 )

#define NODE_DRAW_COLOR_SELECTED_TITLE Color( 75, 75, 90, 255 )
#define NODE_DRAW_COLOR_SELECTED_BOX Color( 220, 150, 110, 255 )
#define NODE_DRAW_COLOR_SELECTED_BORDER Color( 10, 10, 10, 255 )
#define NODE_DRAW_COLOR_SELECTED_TEXT_OP Color( 170, 170, 120, 255 )

#define NODE_DRAW_COLOR_HASSOLVER_BOX Color( 110, 118, 110, 255 )
#define NODE_DRAW_COLOR_HASSOLVER_BOX_ALLOC Color( 110, 196, 110, 255 )

#define NODE_DRAW_COLOR_HASCONTAINER_TITLE Color( 70, 70, 100, 255 )
#define NODE_DRAW_COLOR_HASCONTAINER_BOX Color( 110, 110, 148, 255 )

#define NODE_DRAW_COLOR_CUSTOMTITLE Color( 160, 150, 180, 255 ) // Color( 120, 110, 140, 255 )

class CBaseNode
{
	friend class CBaseContainerNode;

public:
	DECLARE_CLASS_NOBASE(CBaseNode);

	CBaseNode( const char *opName, CNodeView *p );
	virtual ~CBaseNode();

	virtual void SetName( const char *opName );
	virtual const char *GetName();
	virtual const char *GetUserName();
	virtual int GetFinalTextSize();

	virtual void Spawn();

	const HNODE GetUniqueIndex(){ return m_iUniqueIndex; };

	CNodeView *GetParent(){ return pNodeView; };

	virtual CBaseContainerNode *GetAsContainer(){ return NULL; };
	virtual bool IsSolvable( bool bHierachyUp,
							CUtlVector< CBaseNode* > *hCallerList = NULL,
							CUtlVector< CBaseNode* > *hTested = NULL );

	virtual const int GetAllowedFlowgraphTypes();

private:
	CUtlVector< CBaseContainerNode* > m_hParentContainers;
	bool RecursiveTestContainerError_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed,
										bool &bLeftContainerOnce, const bool &bHierachyUp, CBaseContainerNode *container );
	void ListContainersChronologically_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed,
														CUtlVector< CBaseContainerNode* > &hList );

protected:
	virtual bool InsertToContainer( CBaseContainerNode *container ); // don't call directly
	virtual bool RemoveFromContainer( CBaseContainerNode *container ); // don't call directly

public:
	bool RecursiveTestContainerError( const bool &bHierachyUp, CBaseContainerNode *container ); // true on error
	virtual bool HasContainerParent( CBaseContainerNode *container );

	virtual int GetNumContainers();
	virtual CBaseContainerNode *GetContainer( int idx );

	virtual Vector2D GetContainerSensitiveCenter();
	virtual void ListContainersChronologically( CUtlVector< CBaseContainerNode* > &hList );

	virtual bool CanBeInContainer(){ return true; };
	virtual void UpdateParentContainers();

	virtual int GetNodeType(){ return HLSLNODE_INVALID; };
	virtual const bool IsNodeCrucial(){ return false; };
	virtual bool ShouldErrorOnUndefined(){ return false; };

	virtual int PerNodeErrorLevel(){ return ERRORLEVEL_NONE; };
	virtual int TestJackFlags_In();

	virtual bool VguiDraw( bool bShadow = false );
	virtual void VguiDraw_Jacks( bool bShadow = false );
	virtual void VguiDraw_Preview();

	virtual void OnLeftClick( Vector2D &pos );
	virtual void OnDragStart();
	virtual void OnDrag( Vector2D &delta );
	virtual void OnDragEnd();
	virtual bool MustDragAlone();
	virtual bool IsWithinBounds_Base( const Vector2D &pos );

	virtual Vector2D GetBoundsMin();
	virtual Vector2D GetBoundsMax();
	virtual Vector2D GetBoundsMinNodeSpace();
	virtual Vector2D GetBoundsMaxNodeSpace();
	virtual Vector2D GetSelectionBoundsMinNodeSpace();
	virtual Vector2D GetSelectionBoundsMaxNodeSpace();
	virtual Vector2D GetCenter();
	const virtual Vector4D &GetBoundsFast();

	virtual Vector2D GetBoundsTitleMin();
	virtual Vector2D GetBoundsTitleMax();
	virtual Vector2D GetBoundsBoxMin();
	virtual Vector2D GetBoundsBoxMax();

	virtual void SetPosition( Vector2D vec, bool bCenter = false );
	virtual Vector2D GetPosition();
	virtual Vector2D GetSize();

	virtual void SetSelected( const bool b ){ bSelected = b; };
	virtual void ToggleSelection(){ bSelected = !bSelected; };
	virtual const bool IsSelected(){ return bSelected; };

	virtual int GetHierachyType();
private:
	virtual int GetHierachyTypeIterateFullyRecursive_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed, int *iAllowances = NULL );
	virtual void CheckHierachyTypeFullyRecursive_Internal( int t, CUtlVector< CBaseNode* > &m_hNodesProcessed );
	//virtual void OnUpdateHierachy_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed, CBridge *pBridgeInitiator, CJack *pCaller );
	virtual void OnUpdateHierachy_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed );
											//CUtlVector< CBridge* > &m_hBridgesToUpdate,
											//CUtlVector< CJack* > &m_hJacksToUpdate );
	virtual void SetTempHierachyTypeFullyRecursive_Internal( CUtlVector< CBaseNode* > &m_hNodesProcessed, int t );

	int m_iTempHierachy;
protected:
	bool m_bAllInputsRequired;
public:
	virtual void OnUpdateHierachy( CBridge *pBridgeInitiator, CJack *pCaller );
	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();
	virtual void SetOutputsUndefined();
	virtual void OnShowSolverErrored();

	virtual void PreSolverUpdated(){};
	virtual void PostSolverUpdated(){};

	virtual int GetHierachyTypeIterateFullyRecursive( int *iAllowances = NULL );
	virtual void CheckHierachyTypeFullyRecursive( int t );
	virtual int GetAllowedHierachiesAsFlags();

	virtual void UpdateBridgeValidity( CBridge *pBridge, CJack *pCaller, int inputErrorLevel );

	virtual const int GetTempHierachyType();
	virtual void SetTempHierachyType( const int t );
	virtual void SetTempHierachyTypeFullyRecursive( const int t );

	//virtual void OnBridgeConnect
	virtual void GenerateJacks_Input( int num );
	virtual void GenerateJacks_Output( int num );
	virtual int GetNumJacks_Out();
	virtual int GetNumJacks_In();
	virtual CJack* GetJack_Out( int i );
	virtual CJack* GetJack_In( int i );
	virtual int GetNumJacks_Out_Connected();
	virtual int GetNumJacks_In_Connected();
	CJack *GetJackByName_Out( const char *name );
	CJack *GetJackByName_In( const char *name );
	CJack *GetJackByResType_Out( int restype );
	CJack *GetJackByResType_In( int restype );
	virtual bool JacksAllConnected_Out();
	virtual bool JacksAllConnected_In();
	virtual void JackHierachyUpdate_Out();
	virtual CHLSL_Var *GetInputToWriteTo( int varTypes );

	virtual void PurgeBridges( bool bInputs = true, bool bOutputs = true );
	enum BridgeRestoreMode
	{
		BY_INDEX = 0,
		BY_NAME,
		BY_RESTYPE,
		BY_COUNT,
	};
	void CreateBridgeRestoreData_Out( BridgeRestoreMode mode, CUtlVector< BridgeRestoreInfo* > &m_hList );
	void CreateBridgeRestoreData_In( BridgeRestoreMode mode, CUtlVector< BridgeRestoreInfo* > &m_hList );
	void RestoreBridgesFromList_Out( CUtlVector< BridgeRestoreInfo* > &m_hList );
	void RestoreBridgesFromList_In( CUtlVector< BridgeRestoreInfo* > &m_hList );

	virtual void UpdateSize();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues( KeyValues *pKV );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV ){};
	virtual void RestoreFromKeyValues_CreateBridges( KeyValues *pKV );

private:
	bool RecursiveFindNode_Internal( CUtlVector< CBaseNode* > &m_hList, CBaseNode *n, bool bHierachyUp );
	void Recursive_AddTailNodes_Internal(	CUtlVector< CBaseNode* > &m_hProcessedNodes,
											CUtlVector< CBaseNode* > &m_hList,
											bool bHierachyUp, bool bOnlyWithoutSolver, CBaseContainerNode *pContainer, bool bAddContainers );
public:
	bool RecursiveFindNode( CBaseNode *n, bool bHierachyUp );
	void Recursive_AddTailNodes( CUtlVector< CBaseNode* > &m_hList, bool bHierachyUp,
					bool bOnlyWithoutSolver = false, CBaseContainerNode *pContainer = NULL, bool bAddContainers = false );

	const int GetErrorLevel();
	void SetErrorLevel( const int e );

	virtual const bool IsPreviewEnabled(){ return m_bPreviewEnabled; };
	virtual bool IsPreviewVisible();
	void UpdatePreviewAllowed();

	void SetAllocating( const bool &a );
	bool IsAllocating();

protected:
	bool m_bIsAllocating;

	bool m_bAllowPreview;
	bool m_bPreviewEnabled;
	float m_flMinSizeX;
	float m_flMinSizeY;
	float m_flMinSizeY_VS;
	float m_flMinSizePREVIEW_X;

	HNODE m_iUniqueIndex;
	static HNODE m_iUniqueIndexCount;

	Vector2D m_vecBorderInfo;
	Vector2D m_vecPosition;
	Vector2D m_vecSize;
	Vector4D m_vecBounds;

	char szOpName[MAX_PATH];
	char szNodeName[MAX_PATH];

	CNodeView *pNodeView;

	bool bSelected;
	int iErrorLevel;

	CUtlVector< CJack* >m_hInputs;
	CUtlVector< CJack* >m_hOutputs;
	void TouchJacks();

	void SetJackFlags( CJack *j, HLSLJackVarCluster mode );
	void SetJackFlags_Input( int idx, HLSLJackVarCluster mode );
	void SetJackFlags_Output( int idx, HLSLJackVarCluster mode );
	void SetJackFlags_MinFloatRequirement( CJack *j, int components );
	void SetJackFlags_Input_Flags( int idx, int Flags );
	void SetJackFlags_Output_Flags( int idx, int Flags );

	void SetupJackOutput( int idx, HLSLJackVarCluster mode, const char *name );
	void SetupJackInput( int idx, HLSLJackVarCluster mode, const char *name );

	// these lock smarttype!!!
	void LockJackOutput_Flags( int idx, int Flag, const char *name = NULL );
	void LockJackInput_Flags( int idx, int Flag, const char *name = NULL );

public:
	void AddSolver( CHLSL_SolverBase *solver );
	virtual void RemoveSolvers( bool bDelete = true );
	bool InvokeCreateSolvers( GenericShaderData *ShaderData );
	virtual void SmartCreateDummySolver();

	const CUtlVector< CHLSL_SolverBase* > *GetSolvers();
	CHLSL_SolverBase *GetSolver( int idx );
	const int GetNumSolvers();
	bool HasDummySolvers();

	void SweepJackHlslCache();

	void MarkForDeletion();
	const bool IsMarkedForDeletion();

protected:
	virtual bool CreateSolvers( GenericShaderData *ShaderData );
	CUtlVector< CHLSL_SolverBase* > m_hLocalSolvers;

	int m_iPreviewMaterial;

private:

	bool m_bMarkedForDeletion;
};



#endif