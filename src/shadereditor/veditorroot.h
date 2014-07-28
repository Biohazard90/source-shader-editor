#ifndef CEditorRoot_H
#define CEditorRoot_H

#include "cbase.h"
using namespace vgui;

#include <vgui_controls/Frame.h>
#include <vgui_controls/label.h>
#include <vgui_controls/textentry.h>
#include "editorCommon.h"
#include <vgui_controls/controls.h>
#include "cSolverCallback.h"

class CNodeView;
class CPreview;

class CEditorRoot : public vgui::Frame, public CAutoSolverCallback
{
	DECLARE_CLASS_SIMPLE( CEditorRoot, vgui::Frame );

	friend class CDialog_EditorConfig;
	friend class CDialogRecompileAll;
	friend class CShaderPrecache;

public:
	CEditorRoot( const char *pElementName );
	~CEditorRoot();

	virtual void OnThink();
	virtual bool ShouldDraw( void );
	void	Init( void );
	void	LevelInit( void );
	void	LevelShutdown();
	void	OnSceneRender();
	void	OnPostRender();
	void	SetVisible(bool state);

	void OnNewFile();

	virtual bool IsOpaque(){ return true; };

	virtual void OnKeyCodeTyped(KeyCode code);
	virtual void OnKeyCodePressed ( vgui::KeyCode code );
	virtual void OnKeyCodeReleased( vgui::KeyCode code );

	virtual void OnMousePressed(MouseCode code);

	virtual void OnMouseCaptureLost();

	const HFont &GetFont1( int idx ){
		Assert(bFontsLoaded);
		return hFonts[idx];
	};
	const HFont &GetFont2( int idx ){
		Assert(0);
		static HFont NullHFont = 0;
		return NullHFont;
		//Assert(bFontsLoaded);
		//return hFonts2[idx];
	};
	const HFont &GetMarlettFont(){ return _Font_Marlett; };

	const bool ShouldDraw_Datatypes();
	const bool ShouldDraw_Shadows();
	const bool ShouldDraw_Nodes();
	const bool ShouldDraw_Jacks();
	const bool ShouldDraw_Bridges();
	const bool ShouldDraw_AllLimits();
	const bool ShouldAutoCompile(){ return m_bAutoCompile; };
	const bool ShouldAutoFullcompile(){ return m_bAutoFullcompile; };
	const bool ShouldAutoPublish(){ return m_bAutoShaderPublish; };
	const bool ShouldAutoUpdatePrecache(){ return m_bAutoPrecacheUpdate; };
	const bool ShouldShowTooltips(){ return m_bDoTooltips; };
	
	void SendCopyNodes( KeyValues *pKV );
	KeyValues *GetCopyNodes();

	void ToggleFullScreen();
	void ToggleVisible();
	void ToggleInput();

	const bool HasInputEnabled(){ return m_bHasInput; };

	void LoadLayout();
	void SaveLayout();

	void SaveAllGraphs();
	CNodeView *GetMainFlowgraph();
	CNodeView *GetSafeFlowgraph();

	const int GetNumFlowGraphs();
	CNodeView *GetFlowGraph( int i );
	CNodeView *GetActiveFlowGraph();
	CFlowGraphPage *GetActiveFlowgraphPage();

	MESSAGE_FUNC_INT( OnMenuFile, "onmenufile", entry );
	MESSAGE_FUNC_INT( OnMenuShader, "onmenushader", entry );
	MESSAGE_FUNC_INT( OnMenuPostProcessing, "onmenupostprocessing", entry );
	
	MESSAGE_FUNC_PARAMS( OnFileSelected, "FileSelected", pKV );
	MESSAGE_FUNC_PARAMS( OnMenuOpen, "MenuOpen", pKV );

	MESSAGE_FUNC( PageChanged, "PageChanged" );
	MESSAGE_FUNC_PARAMS( OnAskPageClose, "AskPageClose", pKV );
	MESSAGE_FUNC_PARAMS( OnResponseGraphSave, "ResponseGraphSave", pKV );
	MESSAGE_FUNC_INT( PageClosed, "PageClosed", pagenum );
	MESSAGE_FUNC( RequestAddTab, "RequestAddTab" );

	const char *GetCurrentShaderName(); //{ return m_szShaderName; };
	void SetCurrentShaderName( const char *n );
	const char *GetDefaultTabName( CNodeView *pView );

	void OpenShaderFlowgraph( int mode, bool bSM20, bool bForceNewTab = false );
	void UpdateTabColor( CNodeView *pView = NULL );

	//void CompileCallback( bool bError, HCURSTACKIDX idx );

	void RefreshNodeInstances( int iNodeType, KeyValues *pParams );

	KeyValues *GetNodeHelpContainer();

protected:

   virtual void Paint();
   void OnCommand(const char* pcCommand);
   virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
   virtual void PerformLayout();

   void GenerateFonts( vgui::IScheme *pScheme );

private:
	CNodeView *AddNewTab( CNodeView::FlowGraphType_t mode = CNodeView::FLOWGRAPH_HLSL, bool bActivate = true );
	void ConfirmTabClose( vgui::VPANEL panel );

	void InitColors();
	

	Label *m_pLabelTitle;

	void SortButtons();
	//bool m_bNeedsButtonPush;
	void TakeScreenshot();

	bool m_bHalfView;
	bool m_bHasInput;
	int Mx, My;
	vgui::MenuBar *m_pMenuBar;
	vgui::MenuButton *m_pMBut_File;
	vgui::MenuButton *m_pMBut_Shader;
	vgui::MenuButton *m_pMBut_PostProc;
	DHANDLE< FileOpenDialog >	m_hShaderBrowser;

	//HCURSTACKIDX m_iLastCompileIndex;

	KeyValues *m_pKV_NodeHelp;

	//CUtlVector< CFlowGraphPage* > m_hFlowGraphPages;
	CFlowGraphSheet *pNodeSheet;
	//CFlowGraphPage *pPageMain;
	//CNodeView *pNodeView;
	CPreview *pPreview;
	//CUtlVector< CNodeView* >m_hNodeViews;

	void OpenFileDialog( bool bSave );
	void CreatePreview();

	Label *m_pLabel_Coords;
	Label *m_pLabel_FrameTime;
	//Label *m_pLabel_CurrentFileName;

	bool bFontsLoaded;
	HFont hFonts[MAX_ZOOM_FONTS];
	HFont hFonts2[MAX_ZOOM_FONTS];
	HFont _Font_Marlett;
	//char m_szShaderName[MAX_PATH];
	void OnShaderNameChanged();

	KeyValues *m_pKV_SelectionCopy;
	int px, py, psx, psy;
	int cedit_x, cedit_y, cedit_sx, cedit_sy;

	bool m_bDraw_Datatypes;
	bool m_bDraw_Shadows;
	bool m_bDraw_AllLimits;
	bool m_bAutoCompile;
	bool m_bAutoFullcompile;
	bool m_bAutoShaderPublish;
	bool m_bAutoPrecacheUpdate;
	bool m_bDoTooltips;
	bool m_bWarnOnClose;

public:

	bool GetCodeEditorBounds( int &x, int &y, int &sx, int &sy );
	void SetCodeEditorBounds( int x, int y, int sx, int sy );

	IMaterial *GetOperatorMaterial( int i );
	IMaterialVar *GetUVTargetParam( int i, int num );
	//IMaterialVar *GetOperatorParam( int i );

	void AllocProceduralMaterials();
	void UpdateVariablePointer();
	void DeleteProceduralMaterials();

	IMaterial *GetMainPreviewMat(); //{ Assert(m_pMat_MainPreview); return m_pMat_MainPreview; };
	IMaterial *GetBGPreviewMat(); //{ Assert(m_pMat_BGPreview); return m_pMat_BGPreview; };

	KeyValues *GetMainPreviewKV(); //{ Assert(m_pKV_MainPreviewMat); return m_pKV_MainPreviewMat; };
	KeyValues *GetBGPreviewKV(); //{ Assert(m_pKV_BGPreviewMat); return m_pKV_BGPreviewMat; };

	const char *GetEnvmapOverride();

	void SendFullyCompiledShader( GenericShaderData* data );
	GenericShaderData *GetLastFullyCompiledShader(){ return m_pLastFullCompiledShader; };
	void InvokeShaderToScene( GenericShaderData *pShader = NULL );

	void BeginPaintWorld();
	void EndPaintWorld( bool bValid = false );
	const bool IsPainting(){ return m_bPainting; };

	const int &GetNumReloadMaterials(){ return m_iNumMaterials; };
	char **GetReloadMaterialList(){ return m_pszMaterialList; };

private:

	bool m_bPainting;

	int m_iNumMaterials;
	char **m_pszMaterialList;

	KeyValues *m_pKV_NodePreview[NPSOP_CALC_LAST];
	IMaterial *m_pMat_NodePreview[NPSOP_CALC_LAST];

	IMaterialVar *m_pMatVar_NodePv_UVs[NPSOP_CALC_LAST][6];

	//KeyValues *m_pKV_MainPreviewMat;
	//IMaterial *m_pMat_MainPreview;
	//KeyValues *m_pKV_BGPreviewMat;
	//IMaterial *m_pMat_BGPreview;

	::GenericShaderData *m_pLastFullCompiledShader;
};




#endif