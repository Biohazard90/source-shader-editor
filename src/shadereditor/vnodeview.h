#ifndef CNODEVIEW_H
#define CNODEVIEW_H

#include "cbase.h"
using namespace vgui;

#include <vgui_controls/Frame.h>
#include <vgui_controls/label.h>
#include <vgui_controls/textentry.h>
#include "cSolverCallback.h"

class CEditorRoot;
class CBaseContainerNode;
class CNodePropertySheet;
class CSmartTooltip;

struct NHistoryEntry_t
{
public:
	NHistoryEntry_t( KeyValues *k, const char *n )
	{
		pKV = k;
		Q_memset( szShaderName, 0, sizeof( szShaderName ) );
		if ( n )
			Q_snprintf( szShaderName, sizeof( szShaderName ), "%s", n );
	};
	NHistoryEntry_t()
	{
		pKV = NULL;
		Q_memset( szShaderName, 0, sizeof( szShaderName ) );
	};
	~NHistoryEntry_t()
	{
		if ( pKV )
			pKV->deleteThis();
		pKV = NULL;
	};
	KeyValues *pKV;
	char szShaderName[MAX_PATH];

private:
	NHistoryEntry_t( const NHistoryEntry_t &o );
};

class CNodeView : public vgui::Panel, public CAutoSolverCallback
{
	DECLARE_CLASS_SIMPLE( CNodeView, vgui::Panel );

public:

	enum FlowGraphType_t
	{
		FLOWGRAPH_HLSL = ( 1 << 0 ),
		FLOWGRAPH_HLSL_TEMPLATE = ( 1 << 1 ),
		FLOWGRAPH_POSTPROC = ( 1 << 2 ),
	};

	CNodeView( Panel *parent, CEditorRoot *root, /*FlowGraphType_t type,*/ const char *pElementName );
	~CNodeView();

	FlowGraphType_t GetFlowgraphType();
	void SetFlowgraphType( FlowGraphType_t type, bool bValidateGraphData = true );
	bool ShouldCallCompiler();

	virtual void OnThink();
	void	Init( void );

	static bool bRenderingScreenShot;
	static Vector4D vecScreenshotBounds;

	void PurgeCanvas( bool OnShutdown = false );
	void InitCanvas( FlowGraphType_t type = FLOWGRAPH_HLSL, GenericShaderData *newData = NULL );

	void OnBridgeRemoved( CBridge *b );
	void OnJackRemoved( const CJack *j );

	void OnContainerRemoved( CBaseContainerNode *container );
	void OnContainerAdded( CBaseContainerNode *container );
	void ListContainerAtPos( Vector2D &pos, CUtlVector< CBaseContainerNode* > &hList ); // in node space

private:
	CUtlVector< CBaseContainerNode* > m_hContainerNodes;
	int m_iCursorLast;

	FlowGraphType_t m_FlowGraphType;

public:
	bool IsMouseOver();

	virtual void OnKeyCodeTyped(KeyCode code);
	virtual void OnKeyCodePressed ( vgui::KeyCode code );
	virtual void OnKeyCodeReleased( vgui::KeyCode code );
	virtual void OnParentKeyCodeTyped(KeyCode code);
	virtual void OnParentKeyCodePressed ( vgui::KeyCode code );
	virtual void OnParentKeyCodeReleased( vgui::KeyCode code );

	virtual void OnMousePressed ( vgui::MouseCode code );
	virtual void OnMouseDoublePressed(MouseCode code);
	virtual void OnMouseReleased( vgui::MouseCode code );
	virtual void OnCursorMoved( int x, int y );
	virtual void OnMouseWheeled( int delta );

	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	void BeginDrag( bool bNodes );
	bool IsInDrag();
	void StopDrag( bool bNodes );

	void HandleSelectionInputPressed( CBaseNode *pNode );
	void HandleSelectionInputReleased( CBaseNode *pNode );
	void HandleSelectionInputReleasedBox();

	CBaseNode *GetNodeUnderCursor();
	CBaseNode *GetNodeUnderPos(Vector2D pos);
	CJack *GetJackUnderCursor( float *tolerance = NULL, bool bInputs = true, bool bOutputs = true );
	CJack *GetJackUnderPos(Vector2D pos, float *tolerance = NULL, bool bInputs = true, bool bOutputs = true );

	int GetNodeIndex( CBaseNode *n );
	CBaseNode *GetNodeFromIndex( int idx );
	CBaseNode *GetNodeFromType( int type );

	float GetZoomScalar();
	void SetZoomScalar( float zoom );
	void ToNodeSpace( Vector2D &pos );
	void ToPanelSpace( Vector2D &pos );
	void GetGraphBoundaries( Vector4D &out );

	void ResetView();
	void ResetView_User( bool bInitial = false );

	Vector2D GetMousePosInNodeSpace();

	HFont GetFontScaled( float Scale, bool &bVis, float *zoom = NULL );
	HFont GetFontScaledNoOutline( float Scale, bool &bVis, float *zoom = NULL );

	const bool ShouldDraw_Datatypes();
	const bool ShouldDraw_Shadows();
	const bool ShouldDraw_Nodes();
	const bool ShouldDraw_Jacks();
	const bool ShouldDraw_Bridges();

	MESSAGE_FUNC_PARAMS( OnMenuClosed, "MenuClose", pData );

	virtual void Paint();

protected:

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();

private:

	void InitColors();

	float m_flErrorTime;
	Color _col_Grid;
	Color _col_Vignette;
	Color _col_OutOfDate;
	Color _col_Error;

public:
	Vector2D &AccessViewPos(){ return m_vecPosition; };
	float &AccessViewZoom(){ return m_flZoom; };

private:

	void UpdateMousePos();
	void AccumulateMouseMove();
	void SaveMousePos();
	int Mx, My;
	int MxOld, MyOld;
	int MxDelta, MyDelta;

	long m_lTime_MouseLastMoved;

	Vector2D m_vecPosition;
	Vector2D m_vecPositionGoal;
	float m_flZoom;
	float m_flZoomGoal;
	enum
	{
		DRAG_NONE = 0,
		DRAG_VIEW,
		DRAG_NODES,
		DRAG_SBOX,
		DRAG_BRDIGE,
	};
	int iArmDrag;
	bool bInDrag;

	void CreateTooltip( int iNodeIndex );
	void DestroyTooltip();
	CSmartTooltip *m_pTooltip;

	Vector2D m_vecSelectionBoxStart;

public:
	const int GetNumUserErrorLabels();
	CNodeViewError *GetUserErrorLabel( int index );

	MESSAGE_FUNC_PARAMS( OnDestroyUserError, "DestroyUserError", pKV );

private:
	CUtlVector< CNodeViewError* > m_hUserErrorLabels;
	void UpdateUserErrors();
	void UpdateUserErrorLayouts();

	int m_iTex_Darken;
	void Paint_Grid();
	void Paint_Nodes( bool bShadow );
	void Paint_Bridges( bool bShadow );
	void Paint_SelectionBox();
	void Paint_CompilerIcon();
	void Paint_PreCompilerErrors();
	void Paint_StatusBar();

	void Think_SmoothView();
	void Think_Drag();
	Vector2D Think_PullView();
	void Think_HighlightJacks();
	void Think_UpdateCursorIcon();
	void Think_CreateTooltip();
	void Think_UpdateSolvers();
public:
	bool UpdateSolvers();
private:

	bool IsCursorOutOfBounds( Vector2D *delta = NULL );
	void ForceFocus();

	MESSAGE_FUNC_CHARPTR( OnCommand, "Command", command );
	MESSAGE_FUNC_INT( OnSpawnNode, "spawnNode", type );
	MESSAGE_FUNC_PARAMS( OnOpenProperties, "OpenProperties", data );
	
	Vector2D m_vecMenuCreationPosition;
	void CreateContextMenu( CBaseNode *pMouseOverNode );
	void AddNodesToContextMenu( Menu *pNodeMenu );
	void CreatePropertyDialog( CBaseNode *pNode );

	DHANDLE< Menu >	m_ContextMenu;
	DHANDLE< CNodePropertySheet >	m_Properties;

	bool AllowKeyInput();
	
	CUtlVector< CBaseNode* > m_hNodeList;
	CUtlVector< CBaseNode* > m_hNodesInMove;
	CUtlVector< CBridge* > m_hBridgeList;
	CBaseNode *SpawnNode( int type );

	void DoEnvmapOverride();

public:
	int GetNumNodes(){ return m_hNodeList.Count(); };
	CBaseNode *GetNode( int i ){ Assert(m_hNodeList.IsValidIndex(i)); return m_hNodeList[i]; };

	void MakeSolversDirty();
	void OnHierachyChanged();
	void DoFullHierachyUpdate();
	void DestroyAllSolvers();

	CUtlVector< CHLSL_SolverBase* > &AccessSolverStack_VS();
	CUtlVector< CHLSL_SolverBase* > &AccessSolverStack_PS();
	CUtlVector< CHLSL_SolverBase* > &AccessSolverStack_UNDEFINED();
	CUtlVector< CHLSL_SolverBase* > &AccessSolverStack_POSTPROC();

	void ResetTempHierachy();

	static int CreatePPSolversFromFile( const char *fileName, CUtlVector<CHLSL_SolverBase*> &hOutput, GenericPPEData &config );
	GenericPPEData *AccessPPEConfig();

private:
	bool m_bDirtySolvers;
	void InvokeCreateSolvers( CUtlVector< CBaseNode* > &m_hNodeBottomList,
							CUtlVector< CBaseNode* > &m_hNodeOutList,
							CUtlVector< CBaseContainerNode* > &m_hContainerStack,
							GenericShaderData *ShaderData );

	CUtlVector< CHLSL_SolverBase* >m_hSolverStack_VertexShader;
	CUtlVector< CHLSL_SolverBase* >m_hSolverStack_PixelShader;
	CUtlVector< CHLSL_SolverBase* >m_hSolverStack_NoHierachy;

	CUtlVector< CHLSL_SolverBase* >m_hSolverStack_PostProcessing;

	bool IsSelectionBiggerThanOne();
	CBaseNode *GetFirstSelectedNode();
	void DeselectAllNodes( CBaseNode *pIgnore = NULL );
	void SelectNodes( CUtlVector< CBaseNode* > &m_hNodes, bool bAdd = true );
	void DragSelection( Vector2D delta );
	void MoveNodeToFront( CBaseNode *p );
	void CopySelection();
	void PasteSelection();
	void DeleteSelection();
	void DisconnectSelection();

	int m_iTex_BusyIco;

	void StartOverrideIndices( CUtlVector< CBaseNode* > &m_hNodes );
	void FinishOverrideIndices();
	KeyValues *CopyNodes( CUtlVector< CBaseNode* > &m_hNodesToCopy, bool bCenterNodes = true );
	void PasteNodes( KeyValues *pKV, bool bAutoSelect, Vector2D *origin = NULL, CUtlVector< CBaseNode* > *m_hNewNodes = NULL );
	CBaseNode *AllocateSingleNode( KeyValues *pKV );
	void ConnectBridges( CUtlVector< CBaseNode* > &m_hInstantiatedNodes, KeyValues *pKV );

	void MoveNodes( Vector2D offset, KeyValues *pKV );
	void MoveNodes( Vector2D offset, CBaseNode *n );
	void MoveNodes( Vector2D offset, CUtlVector< CBaseNode* > &m_hList );

	CJack *pJackLast;
	CBridge *m_pCurBridge;
	bool bOverrideNodeIndices;
	CUtlVector< CBaseNode* > m_hTmpNodeIndex;

public:
	void PurgeSolverClients();
	void AddSolverClient( ISolverRequester *client );
	void RemoveSolverClient( ISolverRequester *client );

	void SaveToFile( const char *fullpath = NULL );
	bool LoadFromFile( const char *fullpath, bool bSuppressError = false );


	KeyValues *BuildFullCanvas();
	void RestoreFullCanvas( KeyValues *pCanvas );

	GenericShaderData &GetDataForModify();

	void UpdatePPECache();

private:
	void OnSolvingFinsihed();
	void UpdateSolverClients();
	void CleanupNodes();

	CUtlVector< ISolverRequester* > m_hSolverClients;

	GenericShaderData *m_ShaderData;
	void ClearShaderData();
	void GenerateShaderData();
	void PrepShaderData();
	void CleanupShaderData();

	GenericPPEData *m_PPEData;
	void ClearPPEData();
	void GeneratePPEData();

public:
	void CreateDragBridge( CJack *j );
	void UpdateDragBridge( CJack *j, CBaseNode *n );
	void FinishDragBridge( CJack *j );
	void AddBridgeToList( CBridge *b );

	const HCURSTACKIDX &GetStackIndex(){ return m_StackIndex; };
	const HCURSTACKIDX &GetCompiledStackIndex(){ return m_StackIndex_LastCompiled; };
	const bool IsSaveDirty(){ return m_HistoryIndex != m_HistoryIndex_LastSaved; };
	void UnDirtySave(){ m_HistoryIndex_LastSaved = m_HistoryIndex; };
	const bool IsLastCompileSuccessful(){ return m_bLastCompileSuccessFull; };

	virtual void OnCompilationEnd( int flags, int exitcode, GenericShaderData *data );
	void UploadPreviewData();

private:
	void KillDragBridge();
	CJack *GetBridgeSnapJack();

	CEditorRoot *pEditorRoot;

	HCURSTACKIDX m_StackIndex;
	HCURSTACKIDX m_StackIndex_LastCompiled;
	HCURSTACKIDX m_HistoryIndex;
	HCURSTACKIDX m_HistoryIndex_LastSaved;
	bool	m_bLastCompileSuccessFull;

	KeyValues *pKV_Mat_PsIn;
	IMaterial *m_pMat_PsIn;
	void UpdatePsinMaterial( const char *vsName, GenericShaderData *data = NULL );
	IMaterialVar *m_pMatVar_PsIn_Operation;

	IMaterial *m_pPreview_Material;
	KeyValues *m_pPreview_KV;
	IMaterial *m_pPrevBackground_Material;
	KeyValues *m_pPrevBackground_KV;

	GenericShaderData *m_pLastPreviewData;
	int m_iLastPreviewFlags;

	char m_szEnvmap[MAX_PATH];
	char m_szShaderName[MAX_PATH];
	//ITexture *m_pTexCubemap;
	bool m_bRenderBackground;

	void InitPreviewMaterials();
	void DestroyPreviewMaterials();

public:
	void SetEnvmap( const char *pszPath );
	const char *GetEnvmap();
	void SetShadername( const char *pszPath );
	const char *GetShadername();
	void SetUsingBackgroundMaterial( bool bUse );
	const bool IsUsingBackgroundMaterial();
	//ITexture *GetEnvmapTexture();

	IMaterial *GetPreviewMeshMaterial();
	IMaterial *GetPreviewBackgroundMaterial();
	KeyValues *GetKVMeshMaterial();
	KeyValues *GetKVBackgroundMaterial();

	GenericShaderData *GetPreviewData();

private:

	void UpdateRTs();
	void RenderSolvers( CUtlVector< CHLSL_SolverBase* > &hSolvers );

	bool m_bPreviewMode;
	CUtlVector< LimitReport_t* > hErrorList;

public:
	void SetPreviewMode( bool bEnabled );
	bool IsPreviewModeEnabled();
	void UpdatePsinMaterialVar();

	static void RenderSingleSolver( Preview2DContext &c, IMaterial *pMat );


	enum HistoryActions_t
	{
		HACT_UNDO = 0,
		HACT_REDO,

		HACT_PUSH,
	};
	void MakeHistoryDirty();
	void HistoryAction( HistoryActions_t action );
	void FlushHistory( bool bPast = true, bool bFuture = true);
	int GetNumHistoryEntires_Undo();
	int GetNumHistoryEntires_Redo();

private:

	bool m_bHistoryDirty;

	void Think_UpdateHistory();

	CUtlVector< NHistoryEntry_t* >m_hNodeViewHistory;
	CUtlVector< NHistoryEntry_t* >m_hNodeViewHistory_Redo;

	void ApplyHistoryEntry( NHistoryEntry_t *pEntry );
	NHistoryEntry_t *AllocHistoryEntry();
};




#endif