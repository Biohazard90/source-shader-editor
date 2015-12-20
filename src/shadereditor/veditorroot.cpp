#include "cbase.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include "vgui/ISurface.h"
#include "ienginevgui.h"
#include <vgui_controls/controls.h>
#include <vgui_controls/button.h>
#include <vgui_controls/textentry.h>
#include <vgui_controls/richtext.h>
#include <vgui_controls/propertysheet.h>
#include <vgui_controls/propertypage.h>
#include <vgui_controls/menu.h>
#include <vgui_controls/menubutton.h>
#include <vgui_controls/menubar.h>

#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystem.h"

#include "editorcommon.h"
#include "veditorroot.h"
#include "vnodeview.h"

//#include "vgui_int.h"

CEditorRoot *pEditorRoot = NULL;



enum ERoot_FileMenu_t
{
	ER_FMENU_NEW = 0,
	ER_FMENU_OPEN,
	ER_FMENU_SAVE,
	ER_FMENU_SAVE_AS,
	ER_FMENU_SAVE_ALL,
	ER_FMENU_SCREENSHOT,
	ER_FMENU_ECONFIG,
	ER_FMENU_UNDO,
	ER_FMENU_REDO,
};
enum ERoot_ShaderMenu_t
{
	ER_SMENU_SCONFIG = 0,
	ER_SMENU_FULLCOMPILE,
	ER_SMENU_ALLCOMPILE,
	ER_SMENU_KILLCOMPILER,
	ER_SMENU_SPRECACHE,
	ER_SMENU_INJECT,
	ER_SMENU_PAINT,
};
enum ERoot_PostProcMenu_t
{
	ER_PMENU_PCONFIG = 0,
	ER_PMENU_PPRECACHE,
	ER_PMENU_RTS,
	ER_PMENU_,
};

CEditorRoot::CEditorRoot( const char *pElementName ) : BaseClass( NULL, "editor" )
{
	pEditorRoot = this;
	Q_memset( hFonts, 0, sizeof(hFonts) );
	Q_memset( hFonts2, 0, sizeof(hFonts2) );
//	m_pKV_MainPreviewMat = NULL;
	//m_pMat_MainPreview = NULL;
//	m_pKV_BGPreviewMat = NULL;
	//m_pMat_BGPreview = NULL;
	m_bPainting = false;
	m_bAutoCompile = true;
	m_bAutoFullcompile = false;
	m_bAutoShaderPublish = true;
	m_bAutoPrecacheUpdate = true;
	m_bWarnOnClose = true;

	InitColors();

	pPreview = NULL;
	m_pKV_NodeHelp = NULL;

	m_iNumMaterials = 0;
	m_pszMaterialList = NULL;

	m_pLastFullCompiledShader = NULL;

	//vgui::VPANEL GameUIRoot = enginevgui->GetPanel( PANEL_GAMEUIDLL );
	vgui::VPANEL EngineRoot = enginevgui->GetPanel( PANEL_ROOT );
	//vgui::VPANEL DLLRoot = VGui_GetClientDLLRootPanel();

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme( scheme );

	m_pLabelTitle = new Label( this, "title_label", "" );

	pNodeSheet = new CFlowGraphSheet( this, "flowgraphsheets", true, true );
	pNodeSheet->AddActionSignalTarget( this );
	pNodeSheet->SetAddTabButtonEnabled( true );
	//pNodeSheet->ShowContextButtons(true);
	/*pNodeView =*/
	AddNewTab();

	//CFlowGraphPage *page = new CFlowGraphPage( this );
	//m_hFlowGraphPages.AddToTail( page );
	//CNodeView *view = new CNodeView( page, this, "main_nodeview" );
	//m_hNodeViews.AddToTail( pNodeView );
	//page->SetFlowGraph( pNodeView );
	//pNodeSheet->AddPage( page, GetDefaultTabName() );

	m_pLabel_Coords = new Label( this, "mouse_node_coords", "" );
	m_pLabel_FrameTime = new Label( this, "framespeed", "" );


	m_pMenuBar = new MenuBar( this, "menu_bar" );
	m_pMBut_File = new MenuButton( this, "mbut_file", "File" );
	m_pMBut_File->AddActionSignalTarget( this );
	Menu *pMenu_File = new Menu( m_pMBut_File, "" );
	pMenu_File->AddMenuItem( "New", new KeyValues("onmenufile","entry",ER_FMENU_NEW), this );
	pMenu_File->AddSeparator();
	pMenu_File->AddMenuItem( "Open", new KeyValues("onmenufile","entry",ER_FMENU_OPEN), this );
	pMenu_File->AddSeparator();
	pMenu_File->AddMenuItem( "Save", new KeyValues("onmenufile","entry",ER_FMENU_SAVE), this );
	pMenu_File->AddMenuItem( "Save as", new KeyValues("onmenufile","entry",ER_FMENU_SAVE_AS), this );
	pMenu_File->AddMenuItem( "Save all", new KeyValues("onmenufile","entry",ER_FMENU_SAVE_ALL), this );
	pMenu_File->AddSeparator();
	pMenu_File->AddMenuItem( "Undo", new KeyValues("onmenufile","entry",ER_FMENU_UNDO), this );
	pMenu_File->AddMenuItem( "Redo", new KeyValues("onmenufile","entry",ER_FMENU_REDO), this );
	pMenu_File->AddSeparator();
	pMenu_File->AddMenuItem( "Take screenshot", new KeyValues("onmenufile","entry",ER_FMENU_SCREENSHOT), this );
	pMenu_File->AddMenuItem( "Editor config", new KeyValues("onmenufile","entry",ER_FMENU_ECONFIG), this );
	m_pMBut_File->SetMenu( pMenu_File );
	m_pMenuBar->AddButton( m_pMBut_File );

	m_pMBut_Shader = new MenuButton( this, "mbut_shader", "Shader" );
	Menu *pMenu_Shader = new Menu( m_pMBut_Shader, "" );
	pMenu_Shader->AddMenuItem( "Shader settings", new KeyValues("onmenushader","entry",ER_SMENU_SCONFIG), this );
	pMenu_Shader->AddMenuItem( "Shader precache", new KeyValues("onmenushader","entry",ER_SMENU_SPRECACHE), this );
	pMenu_Shader->AddSeparator();
	pMenu_Shader->AddMenuItem( "Full compile", new KeyValues("onmenushader","entry",ER_SMENU_FULLCOMPILE), this );
	pMenu_Shader->AddMenuItem( "Compile all precached", new KeyValues("onmenushader","entry",ER_SMENU_ALLCOMPILE), this );
	pMenu_Shader->AddMenuItem( "Terminate compilers", new KeyValues("onmenushader","entry",ER_SMENU_KILLCOMPILER), this );
	pMenu_Shader->AddSeparator();
	pMenu_Shader->AddMenuItem( "Inject shader into world", new KeyValues("onmenushader","entry",ER_SMENU_INJECT), this );
	pMenu_Shader->AddMenuItem( "Paint world", new KeyValues("onmenushader","entry",ER_SMENU_PAINT), this );
	m_pMBut_Shader->SetMenu( pMenu_Shader );
	m_pMenuBar->AddButton( m_pMBut_Shader );

	m_pMBut_PostProc = new MenuButton( this, "mbut_postproc", "Post processing" );
	Menu *pMenu_PostProc = new Menu( m_pMBut_PostProc, "" );
	pMenu_PostProc->AddMenuItem( "Effect settings", new KeyValues("onmenupostprocessing","entry",ER_PMENU_PCONFIG), this );
	pMenu_PostProc->AddMenuItem( "Effect precache", new KeyValues("onmenupostprocessing","entry",ER_PMENU_PPRECACHE), this );
	pMenu_PostProc->AddMenuItem( "Manage rendertargets", new KeyValues("onmenupostprocessing","entry",ER_PMENU_RTS), this );
	m_pMBut_PostProc->SetMenu( pMenu_PostProc );
	m_pMenuBar->AddButton( m_pMBut_PostProc );

	//m_pLabel_CurrentFileName = new Label( this, "filename", "-" );

	LoadControlSettings("shadereditorui/vgui/shadereditor_root.res");
	OnShaderNameChanged();

	Activate();
	SetVisible( true );
	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( true );
	SetPaintEnabled( true );
	SetParent( EngineRoot );

	m_pKV_SelectionCopy = NULL;
	bFontsLoaded = false;

	m_bHalfView = false;
	m_bHasInput = true;
	//m_bNeedsButtonPush = false;

	m_bDraw_Datatypes = true;
	m_bDraw_Shadows = true;
	m_bDraw_AllLimits = true;
	m_bDoTooltips = true;

	//m_szShaderName[0] = '\0';
	px = py = psx = psy = 0;
	cedit_x = cedit_y = cedit_sx = cedit_sy = -1;
	//m_flErrorTime = 0;

	AllocProceduralMaterials();
	UpdateVariablePointer();

	//m_iLastCompileIndex = GetSafeFlowgraph()->GetStackIndex();
}
CEditorRoot::~CEditorRoot()
{
	if ( m_pszMaterialList )
	{
		sEditMRender->DestroyCharPtrList( &m_pszMaterialList );
		m_pszMaterialList = NULL;
		m_iNumMaterials = 0;
	}

	DeleteProceduralMaterials();

	SendCopyNodes( NULL );
	//m_hFlowGraphPages.Purge();
	//m_hNodeViews.Purge();

	if ( pPreview )
	{
		pPreview->SetParent( (Panel*)NULL );
		delete pPreview;
		pPreview = NULL;
	}

	delete m_pLastFullCompiledShader;

	if ( m_pKV_NodeHelp )
		m_pKV_NodeHelp->deleteThis();
	m_pKV_NodeHelp = NULL;
}

KeyValues *CEditorRoot::GetNodeHelpContainer()
{
	if ( !m_pKV_NodeHelp )
	{
		m_pKV_NodeHelp = new KeyValues( "node_help" );
		char tmp[MAX_PATH*4];
		Q_snprintf( tmp, sizeof(tmp), "%s/node_help.txt", GetEditorRootDirectory() );
		if ( !m_pKV_NodeHelp->LoadFromFile( g_pFullFileSystem, tmp ) )
		{
			m_pKV_NodeHelp->deleteThis();
			m_pKV_NodeHelp = NULL;
		}
	}

	return m_pKV_NodeHelp;
}

void CEditorRoot::PageChanged()
{
	SortButtons();

	CNodeView *pView = GetSafeFlowgraph();
	const char *pszShaderName = pView->GetShadername();

	m_pMBut_File->GetMenu()->SetItemEnabled( 2, pszShaderName != NULL );

	const bool bIsShader = pView->GetFlowgraphType() == CNodeView::FLOWGRAPH_HLSL ||
		pView->GetFlowgraphType() == CNodeView::FLOWGRAPH_HLSL_TEMPLATE;
	m_pMBut_Shader->GetMenu()->SetItemEnabled( 0, bIsShader );
	m_pMBut_Shader->GetMenu()->SetItemEnabled( 2, bIsShader );
	m_pMBut_Shader->GetMenu()->SetItemEnabled( 4, bIsShader );
	m_pMBut_Shader->GetMenu()->SetItemEnabled( 5, bIsShader );
	m_pMBut_Shader->GetMenu()->SetItemEnabled( 6, bIsShader );

#ifdef NO_COMPILING
	m_pMBut_Shader->GetMenu()->SetItemEnabled( 2, false );
	m_pMBut_Shader->GetMenu()->SetItemEnabled( 3, false );
	m_pMBut_Shader->GetMenu()->SetItemEnabled( 4, false );
#endif

	const bool bIsPPEffect = pView->GetFlowgraphType() == CNodeView::FLOWGRAPH_POSTPROC;
	m_pMBut_PostProc->GetMenu()->SetItemEnabled( 0, bIsPPEffect );

	if ( pPreview )
	{
		pPreview->UpdateLayout();
	}
}

void CEditorRoot::OnAskPageClose( KeyValues *pKV )
{
	Panel *pPageTab = (Panel*)pKV->GetPtr( "PageTab" );
	CNodeView *pGraph = GetFlowGraph( pKV->GetInt( "TabIndex" ) );

	if ( pPageTab == NULL ||
		pGraph == NULL )
	{
		Assert( 0 );
		return;
	}

	ConfirmTabClose( pPageTab->GetVPanel() );

#if 0
	if ( !m_bWarnOnClose || !pGraph->IsSaveDirty() )
		ConfirmTabClose( pPageTab->GetVPanel() );
	else
	{
		const char *pszShadername = pGraph->GetShadername();

		char shortName[MAX_PATH*4];

		KeyValues *pKVResponse_Yes = new KeyValues( "ResponseGraphSave", "DoSave", 1 );
		KeyValues *pKVResponse_No = new KeyValues( "ResponseGraphSave", "DoSave", 0 );

		if ( pszShadername == NULL )
		{
			pKVResponse_No->deleteThis();
			pKVResponse_No = new KeyValues( "ResponseGraphSave", "DoSave", 2 );
		}
		else
		{
			Q_FileBase( pszShadername, shortName, sizeof( shortName ) );
		}

		pKVResponse_Yes->SetPtr( "PageTab", pPageTab );
		pKVResponse_No->SetPtr( "PageTab", pPageTab );
		pKVResponse_Yes->SetPtr( "Graph", pGraph );
		pKVResponse_No->SetPtr( "Graph", pGraph );

		vgui::PromptSimple *prompt = new vgui::PromptSimple( this, "Unsaved changes" );
		prompt->MoveToCenterOfScreen();

		if ( pszShadername != NULL )
			prompt->SetText( VarArgs( "The graph %s has unsaved changes. Save now?", shortName ) );
		else
			prompt->SetText( "This graph has not been saved yet, are you sure that you want to close it?" );

		prompt->AddButton( "Yes", pKVResponse_Yes );
		prompt->AddButton( "No", pKVResponse_No );
		prompt->MakeReadyForUse();
		prompt->InvalidateLayout( true, true );
	}
#endif
}

void CEditorRoot::OnResponseGraphSave( KeyValues *pKV )
{
	int iResponse = pKV->GetInt( "DoSave" );
	const bool bSave = iResponse == 1;
	const bool bAbort = iResponse == 2;

	if ( bAbort )
		return;

	Panel *pPageTab = (Panel*)pKV->GetPtr( "PageTab" );
	CNodeView *pGraph = (CNodeView*)pKV->GetPtr( "Graph" );

	if ( bSave && pGraph != NULL )
		pGraph->SaveToFile();

	ConfirmTabClose( pPageTab->GetVPanel() );
}

void CEditorRoot::ConfirmTabClose( vgui::VPANEL panel )
{
	ivgui()->PostMessage( panel, new KeyValues("OnCloseConfirmed"), GetVPanel() );
}

void CEditorRoot::PageClosed( int pagenum )
{
	//m_bNeedsButtonPush = true;
}

void CEditorRoot::RequestAddTab()
{
	int mode = ( input()->IsKeyDown( KEY_LCONTROL ) || input()->IsKeyDown( KEY_RCONTROL ) ||
		input()->IsKeyDown( KEY_LSHIFT ) || input()->IsKeyDown( KEY_RSHIFT ) ) ?
				CANVASINIT_PPEFFECT : CANVASINIT_SCRATCH;

	OpenShaderFlowgraph( mode, false, true );
}

void CEditorRoot::SortButtons()
{
	//m_bNeedsButtonPush = false;

	Panel *pViewReset = FindChildByName( "button_viewreset" );

	if ( pViewReset )
	{
		pViewReset->MoveToFront();
		pViewReset->SetBgColor( Color( 127,127,127,127) );
	}
}
void CEditorRoot::OnSceneRender()
{
	if ( pPreview )
		pPreview->OnSceneRender();
}
void CEditorRoot::OnPostRender()
{
	if ( pPreview )
		pPreview->OnPostRender();
}
void CEditorRoot::AllocProceduralMaterials()
{
	for ( int i = 0; i < NPSOP_CALC_LAST; i++ )
	{
		m_pKV_NodePreview[ i ] = new KeyValues( "NODE_CALC" );
		m_pKV_NodePreview[ i ]->SetInt( "$OPERATION", i );
		m_pMat_NodePreview[ i ] = materials->CreateMaterial( VarArgs( "__node_pv_op_%i", i ), m_pKV_NodePreview[ i ] );
		m_pMat_NodePreview[ i ]->Refresh();
		//m_pKV_NodePreview[ i ]->Clear();
	}

	//KeyValues*	m_pKV_MainPreviewMat = new KeyValues( "EDITOR_SHADER" );
	//Assert( !m_pMat_MainPreview );
	//m_pMat_MainPreview = materials->CreateMaterial( "__main_preview", m_pKV_MainPreviewMat );
	//m_pMat_MainPreview->Refresh();

	//KeyValues*	m_pKV_BGPreviewMat = new KeyValues( "BACKGROUND_PREV" );
	//Assert( !m_pMat_BGPreview );
	//m_pMat_BGPreview = materials->CreateMaterial( "__bg_preview", m_pKV_BGPreviewMat );
	//m_pMat_BGPreview->Refresh();
}
void CEditorRoot::UpdateVariablePointer()
{
	bool bFound = false;
	for ( int i = 0; i < NPSOP_CALC_LAST; i++ )
	{
		for ( int a = 0; a < 6; a++ )
		{
			m_pMatVar_NodePv_UVs[ i ][ a ] = m_pMat_NodePreview[ i ]->FindVar( VarArgs( "$INPUT_UVS_%i", a ), &bFound );
			Assert( bFound );
		}
	}

	for ( int i = 0; i < pNodeSheet->GetNumPages(); i++ )
		((CFlowGraphPage*)pNodeSheet->GetPage(i))->GetFlowGraph()->UpdatePsinMaterialVar();

	//for ( int i = 0; i < m_hNodeViews.Count(); i++ )
	//{
	//	m_hNodeViews[i]->UpdatePsinMaterialVar();
	//}
}

IMaterial *CEditorRoot::GetMainPreviewMat()
{
	CNodeView *pView = GetActiveFlowGraph();
	Assert( pView );
	return pView->GetPreviewMeshMaterial();
}

IMaterial *CEditorRoot::GetBGPreviewMat()
{
	CNodeView *pView = GetActiveFlowGraph();
	Assert( pView );
	return pView->GetPreviewBackgroundMaterial();
}

KeyValues *CEditorRoot::GetMainPreviewKV()
{
	CNodeView *pView = GetActiveFlowGraph();
	Assert( pView );
	return pView->GetKVMeshMaterial();
}

KeyValues *CEditorRoot::GetBGPreviewKV()
{
	CNodeView *pView = GetActiveFlowGraph();
	Assert( pView );
	return pView->GetKVBackgroundMaterial();
}

IMaterial *CEditorRoot::GetOperatorMaterial( int i )
{
	Assert( m_pMat_NodePreview[i] );
	return m_pMat_NodePreview[i];
}
IMaterialVar *CEditorRoot::GetUVTargetParam( int i, int num )
{
	Assert( m_pMatVar_NodePv_UVs[i][num] );
	return m_pMatVar_NodePv_UVs[i][num];
}
//IMaterialVar *CEditorRoot::GetOperatorParam( int i )
//{
//	Assert( m_pMatVar_NodePv_OP[i] );
//	return m_pMatVar_NodePv_OP[i];
//}
void CEditorRoot::DeleteProceduralMaterials()
{
	for ( int i = 0; i < NPSOP_CALC_LAST; i++ )
	{
		m_pKV_NodePreview[ i ]->Clear();
		ForceDeleteMaterial( &m_pMat_NodePreview[i] );
		m_pKV_NodePreview[ i ] = NULL;
		//m_pKV_NodePreview[i]->deleteThis();
	}

	//ForceDeleteMaterial( &m_pMat_MainPreview );
	//ForceDeleteMaterial( &m_pMat_BGPreview );

	//m_pMat_MainPreview->Release();
	//m_pMat_MainPreview->DeleteIfUnreferenced();

	//m_pMat_BGPreview->Release();
	//m_pMat_BGPreview->DeleteIfUnreferenced();
}

void CEditorRoot::SendFullyCompiledShader( GenericShaderData* data )
{
	delete m_pLastFullCompiledShader;
	m_pLastFullCompiledShader = new GenericShaderData( *data );

	if ( ShouldAutoPublish() )
		InvokeShaderToScene();
}
void CEditorRoot::InvokeShaderToScene( GenericShaderData *pShader )
{
	if ( pShader == NULL )
		pShader = m_pLastFullCompiledShader;

	if ( pShader == NULL )
		return;

	ReloadGameShaders( pShader );

	for ( int i = 0; i < GetNumFlowGraphs(); i++ )
	{
		if ( GetFlowGraph(i)->GetFlowgraphType() == CNodeView::FLOWGRAPH_POSTPROC )
			GetFlowGraph(i)->MakeSolversDirty();
	}
}

void CEditorRoot::BeginPaintWorld()
{
	m_bPainting = true;
	input()->SetMouseCapture( GetVPanel() );
}

void CEditorRoot::EndPaintWorld( bool bValid )
{
	input()->SetMouseCapture( NULL );
	input()->SetCursorOveride( dc_user );
	m_bPainting = false;

	if ( bValid && !m_pLastFullCompiledShader )
		return;

	if ( input()->GetMouseOver() &&
		( input()->GetMouseOver() == GetVPanel() ||
		ipanel()->HasParent( input()->GetMouseOver(), GetVPanel() ) ) )
		return;

	if ( !sEditMRender )
		return;

	sEditMRender->DestroyCharPtrList( &m_pszMaterialList );
	m_iNumMaterials = sEditMRender->MaterialPicker( &m_pszMaterialList );

	gShaderEditorSystem->BeginMaterialReload( !bValid );
	//ReloadGameShaders( m_pLastFullCompiledShader, m_pszMaterialList, m_iNumMaterials );
}
void CEditorRoot::OnMouseCaptureLost()
{
	if ( m_bPainting )
	{
		m_bPainting = false;
		input()->SetCursorOveride( dc_user );
	}
}
void CEditorRoot::OnMousePressed(MouseCode code)
{
	if ( m_bPainting )
	{
		EndPaintWorld( code == MOUSE_LEFT );
		return;
	}
	BaseClass::OnMousePressed( code );
}

CNodeView *CEditorRoot::GetMainFlowgraph()
{
	return NULL; //pNodeView;
}

CNodeView *CEditorRoot::GetSafeFlowgraph()
{
	CNodeView *pCur = GetActiveFlowGraph();
	Assert( pCur );
	return (pCur) ? pCur : GetMainFlowgraph();
}

const int CEditorRoot::GetNumFlowGraphs()
{
	return pNodeSheet->GetNumPages();
}

CNodeView *CEditorRoot::GetFlowGraph( int i )
{
	CFlowGraphPage *page = (CFlowGraphPage*)pNodeSheet->GetPage( i );
	CNodeView *pView = page ? page->GetFlowGraph() : NULL;
	return pView;
}

CNodeView *CEditorRoot::GetActiveFlowGraph()
{
	CFlowGraphPage *page = GetActiveFlowgraphPage(); //(CFlowGraphPage*)pNodeSheet->GetActivePage();
	CNodeView *pView = page ? page->GetFlowGraph() : NULL;
	return pView;
}

CFlowGraphPage *CEditorRoot::GetActiveFlowgraphPage()
{
	return (CFlowGraphPage*)pNodeSheet->GetActivePage();
}

const bool CEditorRoot::ShouldDraw_Datatypes()
{
	return sedit_draw_datatypes.GetBool() && m_bDraw_Datatypes;
}
const bool CEditorRoot::ShouldDraw_Shadows()
{
	return sedit_draw_shadows.GetBool() && m_bDraw_Shadows;
}
const bool CEditorRoot::ShouldDraw_Nodes()
{
	return sedit_draw_nodes.GetBool();
}
const bool CEditorRoot::ShouldDraw_Jacks()
{
	return sedit_draw_jacks.GetBool();
}
const bool CEditorRoot::ShouldDraw_Bridges()
{
	return sedit_draw_bridges.GetBool();
}
const bool CEditorRoot::ShouldDraw_AllLimits()
{
	return m_bDraw_AllLimits;
}
void CEditorRoot::SendCopyNodes( KeyValues *pKV )
{
	if ( m_pKV_SelectionCopy )
		m_pKV_SelectionCopy->deleteThis();
	if ( pKV )
		m_pKV_SelectionCopy = pKV;
	else
		m_pKV_SelectionCopy = NULL;
}
KeyValues *CEditorRoot::GetCopyNodes()
{
	return m_pKV_SelectionCopy;
}

void CEditorRoot::OnThink(void)
{
	BaseClass::OnThink();

	if ( IsBuildModeActive() )
	{
		Panel *pBuild = FindChildByName( "BuildModeDialog" );
		if ( pBuild && !pBuild->IsCursorOver() && input()->IsKeyDown( KEY_LCONTROL ) )
			pBuild->MoveToFront();
	}

	input()->GetCursorPosition( Mx, My );
	ScreenToLocal(Mx,My);

	Vector2D mCoords = GetSafeFlowgraph()->GetMousePosInNodeSpace();
	m_pLabel_Coords->SetText( VarArgs( "position: %10.1f / %10.1f", mCoords.x, mCoords.y ) );

	static CFastTimer _timer;
	_timer.End();
	double _frametime = _timer.GetDuration().GetSeconds();
	_timer.Start();
	m_pLabel_FrameTime->SetText( VarArgs( "fps: %4.1f, ms: %8.7f", 1.0f/max( 0.00001f, _frametime ), _frametime ) );

	if ( m_bPainting )
		input()->SetCursorOveride( dc_hand );

	// FML
	SortButtons();
	//if ( m_bNeedsButtonPush )
	//	SortButtons();

#ifdef SHADER_EDITOR_DLL_SWARM
	VPANEL modal = input()->GetAppModalSurface();
	static bool bFailed = false;

	if ( !bFailed && modal > 0 && modal != GetVPanel() )
	{
		if ( !m_bHalfView && IsVisible() && !ipanel()->HasParent( modal, GetVPanel() ) )
		{
			const char *pszModalName = ipanel()->GetName( modal );
			const char *names[] = {
				"SteamCloudConfirmation",
				"AddonAssociation",
			};

			const int numnames = ARRAYSIZE( names );
			for ( int i = 0; i < numnames; i++ )
			{
				if ( !Q_stricmp( names[i], pszModalName ) )
				{
					ToggleVisible();
					return;
				}
			}

			bFailed = true;
		}
	}
#endif
}
bool CEditorRoot::ShouldDraw( void )
{
	return true;
}
void CEditorRoot::Init(void)
{
}
void CEditorRoot::LevelInit( void )
{
}
void CEditorRoot::LevelShutdown()
{
}
void CEditorRoot::SetVisible(bool state)
{
	BaseClass::SetVisible(state);
}

//void CEditorRoot::CompileCallback( bool bError, HCURSTACKIDX idx )
//{
//	if (bError)
//		m_flErrorTime = 1;
//	else
//		m_iLastCompileIndex = idx;
//}

void CEditorRoot::InitColors()
{
}

void CEditorRoot::Paint()
{
	BaseClass::Paint();
}

void CEditorRoot::OnMenuFile( int entry )
{
	switch ( entry )
	{
	case ER_FMENU_NEW: // new
			new CDialog_NewCanvas( GetSafeFlowgraph(), this );
		break;
	case ER_FMENU_OPEN: // open
		OpenFileDialog( false );
		break;
	case ER_FMENU_SAVE: // save
		GetSafeFlowgraph()->SaveToFile();
		break;
	case ER_FMENU_SAVE_AS: // save as
		OpenFileDialog( true );
		break;
	case ER_FMENU_SAVE_ALL:
		SaveAllGraphs();
		break;
	case ER_FMENU_SCREENSHOT: // screenshot
		TakeScreenshot();
		break;
	case ER_FMENU_ECONFIG: // editor config
			new CDialog_EditorConfig( this );
		break;
	case ER_FMENU_UNDO:
		{
			CNodeView *pView = GetActiveFlowGraph();
			if ( !pView ) break;
			pView->HistoryAction( CNodeView::HACT_UNDO );
		}
		break;
	case ER_FMENU_REDO:
		{
			CNodeView *pView = GetActiveFlowGraph();
			if ( !pView ) break;
			pView->HistoryAction( CNodeView::HACT_REDO );
		}
		break;
	}
}
void CEditorRoot::OnMenuShader( int entry )
{
	switch ( entry )
	{
	case ER_SMENU_SCONFIG: // settings
			new CDialog_GeneralConfig( GetSafeFlowgraph(), this );
		break;
	case ER_SMENU_FULLCOMPILE: // compile
			if ( !GetSafeFlowgraph() )
				return;
			GetSafeFlowgraph()->SetPreviewMode( false );
			GetSafeFlowgraph()->MakeSolversDirty();
			GetSafeFlowgraph()->UpdateSolvers();
			GetSafeFlowgraph()->SetPreviewMode( true );
		break;
	case ER_SMENU_ALLCOMPILE:
			new CDialogRecompileAll( this );
		break;
	case ER_SMENU_SPRECACHE: // shader precache
			new CShaderPrecache( this, GetSafeFlowgraph() );
		break;
	case ER_SMENU_KILLCOMPILER: // terminate compiler
			ForceTerminateCompilers();
		break;
	case ER_SMENU_INJECT: // inject shader to scene
			InvokeShaderToScene();
		break;
	case ER_SMENU_PAINT: // paint world
			BeginPaintWorld();
		break;
	}
}
void CEditorRoot::OnMenuPostProcessing( int entry )
{
	switch ( entry )
	{
	case ER_PMENU_PCONFIG:
			new CDialog_PPEConfig( this, GetSafeFlowgraph() );
		break;
	case ER_PMENU_PPRECACHE:
			new CDialog_PPEPrecache( this );
		break;
	case ER_PMENU_RTS: // settings
			new CDialog_RendertargetList( this );
		break;
	}
}

void CEditorRoot::TakeScreenshot()
{
	CNodeView *pView = GetActiveFlowGraph();
	if ( !pView )
		return;

	char _path[MAX_PATH*4];
	Q_snprintf( _path, sizeof(_path), "%s\\screenshots", ::GetGamePath() );

	if ( !g_pFullFileSystem->IsDirectory( _path,  "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( _path, "MOD" );

	int idx = 0;
	Q_snprintf( _path, sizeof(_path), "%s\\screenshots\\nodeview_%03i.jpg", ::GetGamePath(), idx );

	while ( g_pFullFileSystem->FileExists( _path ) )
	{
		idx++;
		Q_snprintf( _path, sizeof(_path), "%s\\screenshots\\nodeview_%03i.jpg", ::GetGamePath(), idx );
	}

	Q_snprintf( _path, sizeof(_path), "%s\\screenshots\\nodeview_%03i.jpg", ::GetGamePath(), idx );
	CHLSL_Image::CreateScreenshot( pView, _path );
}

void CEditorRoot::SaveAllGraphs()
{
	for ( int i = 0; i < GetNumFlowGraphs(); i++ )
	{
		if ( GetFlowGraph( i )->GetShadername() )
			GetFlowGraph(i)->SaveToFile( GetFlowGraph(i)->GetShadername() );
	}
}
void CEditorRoot::OnShaderNameChanged()
{
	CNodeView *pView = GetSafeFlowgraph();
	const char *pszShaderName = pView->GetShadername();

	m_pMBut_File->GetMenu()->SetItemEnabled( 2, pszShaderName != NULL );

	Label *pTab = (Label*)pNodeSheet->GetActiveTab();
	Assert( pTab && GetActiveFlowgraphPage() );

	if ( pszShaderName == NULL )
	{
		//m_pLabel_CurrentFileName->SetText( "-" );
		//pView->SetShadername( NULL );

		pTab->SetText( GetDefaultTabName( pView ) );
		//GetActiveFlowgraphPage()->sett
	}
	else
	{
		char tmp[MAX_PATH];
		Q_FileBase( pszShaderName, tmp, sizeof( tmp ) );

		//m_pLabel_CurrentFileName->SetText( tmp );
		pTab->SetText( tmp );
	}

	for ( int i = 0; i < pNodeSheet->GetNumPages(); i++ )
		((Panel*)pNodeSheet->GetTab(i))->InvalidateLayout(true);

	pNodeSheet->InvalidateLayout( true, true );
}
void CEditorRoot::SetCurrentShaderName( const char *n )
{
	CNodeView *pView = GetSafeFlowgraph();
	pView->SetShadername( n );
	//Q_memset( m_szShaderName, 0, sizeof(m_szShaderName) );
	//if ( n )
	//	Q_snprintf( m_szShaderName, sizeof(m_szShaderName), "%s", n );
	OnShaderNameChanged();
}

const char *CEditorRoot::GetCurrentShaderName()
{
	CNodeView *pView = GetSafeFlowgraph();
	return pView->GetShadername();
}

void CEditorRoot::OnMenuOpen( KeyValues *pKV )
{
	Panel *pP = (Panel*)pKV->GetPtr( "panel" );
	if ( pP == m_pMBut_File )
	{
		CNodeView *pView = GetActiveFlowGraph();
		const bool bAllowUndo = pView && pView->GetNumHistoryEntires_Undo() > 1;
		const bool bAllowRedo = pView && pView->GetNumHistoryEntires_Redo() > 0;
		m_pMBut_File->GetMenu()->SetItemEnabled( 5, bAllowUndo );
		m_pMBut_File->GetMenu()->SetItemEnabled( 6, bAllowRedo );
	}
}
void CEditorRoot::OpenFileDialog( bool bSave )
{
	if ( m_hShaderBrowser.Get() )
		m_hShaderBrowser.Get()->MarkForDeletion();

	m_hShaderBrowser = new FileOpenDialog( this,
		bSave ? "Save canvas" : "Load canvas",
		bSave ? FOD_SAVE : FOD_OPEN,
		new KeyValues("FileOpenContext", "context",
		bSave ? "savec" : "openc" )
		);

	if ( m_hShaderBrowser.Get() )
	{
		//m_hShaderBrowser->SetStartDirectory( GetCanvasDirectory() );
		m_hShaderBrowser->SetStartDirectoryContext( GetFODPathContext( FODPC_CANVAS ), GetCanvasDirectory() );
		m_hShaderBrowser->AddFilter( "*.txt", "Canvas", true );
#if MAKE_DEFAULT_CANVAS_EDITABLE
		m_hShaderBrowser->AddFilter( "*.def", "Default canvas", true );
#endif
		m_hShaderBrowser->DoModal( true );
	}
}
void CEditorRoot::OnFileSelected( KeyValues *pKV )
{
	KeyValues *pContext = pKV->FindKey( "FileOpenContext" );
	if ( !pContext )
		return;
	const char *__c = pContext->GetString( "context" );
	bool bSaving = true;
	if ( !Q_stricmp( __c, "openc" ) )
		bSaving = false;

	const char *pathIn = pKV->GetString( "fullpath" );
	if ( Q_strlen( pathIn ) <= 1 )
		return;

	bool bDoViewReset = false;

	if ( !bSaving )
	{
		int iPageIndex = -1;
		for ( int i = 0; i < pNodeSheet->GetNumPages(); i++ )
		{
			const char *pszShadername = ((CFlowGraphPage*)pNodeSheet->GetPage(i))->GetFlowGraph()->GetShadername();
			if ( pszShadername && !Q_stricmp( pszShadername, pathIn ) )
				iPageIndex = i;
		}
		bool bExists = iPageIndex >= 0;

		if ( bExists )
		{
			pNodeSheet->SetActivePage( pNodeSheet->GetPage( iPageIndex ) );
			//return;
		}
		else if ( ( input()->IsKeyDown( KEY_LCONTROL ) || input()->IsKeyDown( KEY_RCONTROL ) ) )
		{
			AddNewTab();
			bDoViewReset = true;
		}
	}
	else
	{
		Panel *p = pNodeSheet->GetActivePage();
		for ( int i = 0; i < pNodeSheet->GetNumPages(); i++ )
		{
			Panel *pCur = pNodeSheet->GetPage(i);
			if (pCur == p)
				continue;

			const char *pszShadername = ((CFlowGraphPage*)pCur)->GetFlowGraph()->GetShadername();
			if ( pszShadername && !Q_stricmp( pszShadername, pathIn ) )
			{
				pNodeSheet->DeletePage(pCur);
				i--;
			}
		}
	}

	SetCurrentShaderName( pathIn );
	CNodeView *pView = GetSafeFlowgraph();

	if ( bSaving )
		pView->SaveToFile( pView->GetShadername() ); //m_szShaderName );
	else
	{
		pView->LoadFromFile( pView->GetShadername() ); //m_szShaderName );

		if ( bDoViewReset )
		{
			pView->InvalidateLayout( true, true );
			pView->ResetView_User( false );
		}

		PageChanged();
	}

	pView->RequestFocus();
}

void CEditorRoot::OpenShaderFlowgraph( int mode, bool bSM20, bool bForceNewTab )
{
	//CNodeView::FlowGraphType_t curType = GetSafeFlowgraph()->GetFlowgraphType();

	CNodeView::FlowGraphType_t targetType = CNodeView::FLOWGRAPH_HLSL;
	switch (mode)
	{
	case CANVASINIT_PPEFFECT:
		targetType = CNodeView::FLOWGRAPH_POSTPROC;
		break;
	}

	bool bPushHistory = false;

	if ( bForceNewTab ||
		input()->IsKeyDown( KEY_LCONTROL ) || input()->IsKeyDown( KEY_RCONTROL ) )
		AddNewTab(targetType);
	else
		bPushHistory = true;

	OnNewFile();

	CNodeView *pView = GetSafeFlowgraph();
	pView->PurgeCanvas();

	const char *FileToLoad = NULL;

	switch (mode)
	{
	default:
	case CANVASINIT_SCRATCH:
		{
			GenericShaderData *data = new GenericShaderData();
			if ( bSM20 )
				data->shader->iShaderModel = SM_20B;

			pView->InitCanvas(targetType,data);
			delete data;
		}
		break;

	case CANVASINIT_MODEL_SIMPLE:
#ifdef SHADER_EDITOR_DLL_SWARM
		FileToLoad = "def_model_simple_swarm.def";
#else
		FileToLoad = "def_model_simple.def";
#endif
		break;

	case CANVASINIT_MODEL_COMPLEX:
#ifdef SHADER_EDITOR_DLL_SWARM
		FileToLoad = "def_model_complex_swarm.def";
#else
		FileToLoad = "def_model_complex.def";
#endif
		break;

	case CANVASINIT_POSTPROC:
		FileToLoad = "def_pp.def";
		break;

	case CANVASINIT_LIGHTMAPPED_SIMPLE:
#ifdef SHADER_EDITOR_DLL_SWARM
		FileToLoad = "def_lightmap_simple_swarm.def";
#else
		FileToLoad = "def_lightmap_simple.def";
#endif
		break;

	case CANVASINIT_LIGHTMAPPED_BUMP:
#ifdef SHADER_EDITOR_DLL_SWARM
		FileToLoad = "def_lightmap_bump_swarm.def";
#else
		FileToLoad = "def_lightmap_bump.def";
#endif
		break;
	case CANVASINIT_PPEFFECT:
		pView->InitCanvas(targetType);
		break;
	}

	if ( FileToLoad )
	{
		char fPath[ MAX_PATH ];
		Q_snprintf( fPath, MAX_PATH, "%s/shadereditorui/canvas_default/%s", GetGamePath(), FileToLoad );
		pView->LoadFromFile( fPath );
	}

	OnShaderNameChanged();
	UpdateTabColor( pView );

	pView->InvalidateLayout( true, true );
	pView->ResetView_User( false );

	if ( bPushHistory )
		pView->MakeHistoryDirty();

	PageChanged();

	pView->UnDirtySave();
}

CNodeView *CEditorRoot::AddNewTab( CNodeView::FlowGraphType_t mode, bool bActivate )
{
	CFlowGraphPage *page = new CFlowGraphPage( this );
	//m_hFlowGraphPages.AddToTail( page );

	CNodeView *pView = new CNodeView( page, this, /*mode,*/ "nodeview_panel" );
	//m_hNodeViews.AddToTail( pView );
	pView->SetFlowgraphType( mode );
	pView->ResetView();

	page->SetFlowGraph( pView );
	pNodeSheet->AddPage( page, GetDefaultTabName( pView ) );

	if ( bActivate )
		pNodeSheet->SetActivePage( page );

	UpdateTabColor( pView );

	return pView;
}

const char *CEditorRoot::GetDefaultTabName( CNodeView *pView )
{
	Assert( pView );
	switch ( pView->GetFlowgraphType() )
	{
	default:
	case CNodeView::FLOWGRAPH_HLSL:
		return "Unknown shader";
	case CNodeView::FLOWGRAPH_POSTPROC:
		return "Unknown post processing effect";
	case CNodeView::FLOWGRAPH_HLSL_TEMPLATE:
		return "Unknown template graph";
	}
}

void CEditorRoot::UpdateTabColor( CNodeView *pView )
{
	if ( !pView )
		pView = GetSafeFlowgraph();

	CNodeView::FlowGraphType_t mode = pView->GetFlowgraphType();

	Color col( 127, 127, 127, 255 );

	switch ( mode )
	{
	case CNodeView::FLOWGRAPH_HLSL:
			col.SetColor( 100, 108, 102, 255 );
		break;
	case CNodeView::FLOWGRAPH_POSTPROC:
			col.SetColor( 108, 104, 100, 255 );
		break;
	default:
			Assert(0);
		break;
	}

	int pageIndex = -1;
	for ( int i = 0; i < pNodeSheet->GetNumPages() && pageIndex < 0; i++ )
		if ( GetFlowGraph( i ) == pView )
			pageIndex = i;

	Assert( pageIndex >= 0 && pageIndex < pNodeSheet->GetNumPages() );
	pNodeSheet->SetTabColor( pageIndex, col );
}

void CEditorRoot::RefreshNodeInstances( int iNodeType, KeyValues *pParams )
{
	const char *pszParam_Path = pParams->GetString( "filepath" );

	for ( int i = 0; i < pNodeSheet->GetNumPages(); i++ )
	{
		CNodeView *pView = GetFlowGraph( i );

		for ( int n = 0; n < pView->GetNumNodes(); n++ )
		{
			CBaseNode *pNode = pView->GetNode( n );

			if ( pNode->GetNodeType() != iNodeType )
				continue;

			switch ( iNodeType )
			{
			case HLSLNODE_UTILITY_CUSTOMCODE:
				{
					CNodeCustom *pCustomNode = (CNodeCustom*)pNode;
					if ( pCustomNode->IsUsingInlineCode() || Q_stricmp( pCustomNode->GetFilePath(), pszParam_Path ) )
						break;

					pCustomNode->UpdateFromFile();
				}
				break;
			}
		}
	}

	pParams->deleteThis();
}

void CEditorRoot::CreatePreview()
{
	if ( !pPreview )
	{
		pPreview = new CPreview( this, NULL ); //pNodeView );
		pPreview->MoveToCenterOfScreen();
		if ( psx > 0 && psy > 0 )
		{
			ClipToScreenBounds( px, py, psx, psy );
			pPreview->SetBounds( px, py, psx, psy );
		}
		else
			pPreview->GetBounds(px,py,psx,psy);

		pPreview->RememberPosition();
	}
	else
	{
		if ( !pPreview->IsVisible() )
			pPreview->Activate();
		else
			pPreview->Close();
		return;
	}

	//int mx, my;
	//pPreview->GetMinimumSize( mx, my );
	//pPreview->SetSize( mx, my );
	pPreview->MakeReadyForUse();
	pPreview->InvalidateLayout( true, true );
}
const char *CEditorRoot::GetEnvmapOverride()
{
	if ( !pPreview )
		return NULL;

	const char *pEnv = pPreview->GetEnvmap();
	if ( !pEnv || !*pEnv )
		return NULL;

	return pEnv;
}
void CEditorRoot::OnNewFile()
{
	//m_szShaderName[0] = '\0';
	CNodeView *pView = GetSafeFlowgraph();
	pView->SetShadername(NULL);
	OnShaderNameChanged();
}
void CEditorRoot::LoadLayout()
{
	KeyValues *pKV = new KeyValues( "config" );
	bool bPreview = false;
	if ( pKV->LoadFromFile( filesystem, VarArgs( "%s/shadereditorui/editor_config.txt", engine->GetGameDirectory()), "MOD" ) )
	{
		px = pKV->GetInt( "p_x" );
		py = pKV->GetInt( "p_y" );
		psx = pKV->GetInt( "p_sx" );
		psy = pKV->GetInt( "p_sy" );
		if ( pKV->GetInt( "preview_visible" ) )
			bPreview = true;
	}
	else
	{
		px=py=psx=psy=0;
	}

	m_bDraw_Datatypes = !!pKV->GetInt( "draw_datatypes", 1 );
	m_bDraw_Shadows = !!pKV->GetInt( "draw_shadows", 1 );
	m_bDraw_AllLimits = !!pKV->GetInt( "draw_all_limits", 0 );
	m_bAutoCompile = !!pKV->GetInt( "compile_preview", 1 );
	m_bAutoFullcompile = !!pKV->GetInt( "compile_always_full", 0 );
	m_bAutoShaderPublish = !!pKV->GetInt( "compile_auto_publish_to_materials", 1 );
	m_bDoTooltips = !!pKV->GetInt( "enable_nodeview_tooltips", 1 );
	m_bAutoPrecacheUpdate = !!pKV->GetInt( "precache_auto_reload", 1 );
	m_bWarnOnClose = !!pKV->GetInt( "warn_unsaved_changes", 1 );

	cedit_x = pKV->GetInt( "cedit_x", -1 );
	cedit_y = pKV->GetInt( "cedit_y", -1 );
	cedit_sx = pKV->GetInt( "cedit_sx", -1 );
	cedit_sy = pKV->GetInt( "cedit_sy", -1 );

	pKV->deleteThis();

	if ( bPreview )
		CreatePreview();
}
void CEditorRoot::SaveLayout()
{
	KeyValues *pKV = new KeyValues( "config" );

	pKV->SetInt( "preview_visible", (pPreview && pPreview->IsVisible()) );
	if ( pPreview )
		pPreview->GetBounds( px, py, psx, psy );

	pKV->SetInt( "p_x", px );
	pKV->SetInt( "p_y", py );
	pKV->SetInt( "p_sx", psx );
	pKV->SetInt( "p_sy", psy );

	pKV->SetInt( "draw_datatypes", m_bDraw_Datatypes ? 1 : 0 );
	pKV->SetInt( "draw_shadows", m_bDraw_Shadows ? 1 : 0 );
	pKV->SetInt( "draw_all_limits", m_bDraw_AllLimits ? 1 : 0 );
	pKV->SetInt( "compile_preview", m_bAutoCompile ? 1 : 0 );
	pKV->SetInt( "enable_nodeview_tooltips", m_bDoTooltips ? 1 : 0 );
	pKV->SetInt( "compile_always_full", m_bAutoFullcompile ? 1 : 0 );
	pKV->SetInt( "compile_auto_publish_to_materials", m_bAutoShaderPublish ? 1 : 0 );
	pKV->SetInt( "precache_auto_reload", m_bAutoPrecacheUpdate ? 1 : 0 );
	pKV->SetInt( "warn_unsaved_changes", m_bWarnOnClose ? 1 : 0 );

	pKV->SetInt( "cedit_x", cedit_x );
	pKV->SetInt( "cedit_y", cedit_y );
	pKV->SetInt( "cedit_sx", cedit_sx );
	pKV->SetInt( "cedit_sy", cedit_sy );

	pKV->SaveToFile( filesystem, VarArgs( "%s/shadereditorui/editor_config.txt", engine->GetGameDirectory()), "MOD" );
	pKV->deleteThis();

	//DeallocPingPongRTs();
}

bool CEditorRoot::GetCodeEditorBounds( int &x, int &y, int &sx, int &sy )
{
	x = y = sx = sy = 50;

	if ( cedit_x < 0 || cedit_y < 0 || cedit_sx < 0 || cedit_sy < 0 )
		return false;

	ClipToScreenBounds( cedit_x, cedit_y, cedit_sx, cedit_sy );

	x = cedit_x;
	y = cedit_y;
	sx = cedit_sx;
	sy = cedit_sy;
	return true;
}

void CEditorRoot::SetCodeEditorBounds( int x, int y, int sx, int sy )
{
	cedit_x = x;
	cedit_y = y;
	cedit_sx = sx;
	cedit_sy = sy;
}

void CEditorRoot::OnCommand(const char* pcCommand)
{
	if( !Q_stricmp(pcCommand, "quit") )
	{
		engine->ClientCmd( "quit" );
		return;
	}
	if( !Q_stricmp(pcCommand, "resetview") )
	{
		CNodeView *pView = GetActiveFlowGraph();
		if ( pView )
			pView->ResetView_User();
		return;
	}
	if( !Q_stricmp(pcCommand, "options") )
	{
		if ( !enginevgui->IsGameUIVisible() )
		{
			vgui::VPANEL GameUIRoot = enginevgui->GetPanel( PANEL_GAMEUIDLL );
			ipanel()->SetVisible( GameUIRoot, true );
		}
		engine->ClientCmd( "gamemenucommand OpenOptionsDialog" );
		return;
	}
	if( !Q_stricmp(pcCommand, "preview") )
	{
		CreatePreview();
		return;
	}
	if( !Q_stricmp(pcCommand, "pv_closed") )
	{
		if (pPreview)
			pPreview->GetBounds( px, py, psx, psy );
		return;
	}
	BaseClass::OnCommand( pcCommand );
}

void CEditorRoot::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(false);
	SetPaintEnabled(true);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);

	InitColors();

	SetTitle( "", true );
	SetTitleBarVisible( false );
	m_pLabelTitle->SetText( GetEditorTitle() );
	m_pLabelTitle->SetFont( pScheme->GetFont( "UiBold", false ) );

	SetPaintBackgroundType( 0 );
	OnShaderNameChanged();
	GenerateFonts(pScheme);
}
void CEditorRoot::PerformLayout()
{
	BaseClass::PerformLayout();

	SetZPos(0);
	int wide,tall;
	surface()->GetScreenSize(wide, tall);
	if ( m_bHalfView )
		wide /= 2;
	SetPos(0,0);
	SetSize(wide,tall);

	SetMouseInputEnabled(m_bHasInput);
	SetKeyBoardInputEnabled(m_bHasInput);

	m_pLabelTitle->SetVisible( !m_bHalfView );
}
void CEditorRoot::OnKeyCodeTyped(KeyCode code)
{
	BaseClass::OnKeyCodeTyped( code );

	CNodeView *pView = GetActiveFlowGraph();
	if ( pView )
		pView->OnParentKeyCodeTyped( code );
}
void CEditorRoot::OnKeyCodePressed ( vgui::KeyCode code )
{
	BaseClass::OnKeyCodePressed( code );

	CNodeView *pView = GetActiveFlowGraph();
	if ( pView )
		pView->OnParentKeyCodePressed( code );
}
void CEditorRoot::OnKeyCodeReleased( vgui::KeyCode code )
{
	BaseClass::OnKeyCodeReleased( code );

	CNodeView *pView = GetActiveFlowGraph();
	if ( pView )
		pView->OnParentKeyCodeReleased( code );
}

void CEditorRoot::GenerateFonts(vgui::IScheme *pScheme)
{
	//if ( bFontsLoaded )
	//	return;

	GetFontCacheHandle()->InvalidateFonts();
	GetFontCacheHandle()->AllocFonts();

	for ( int i = 0; i < MAX_ZOOM_FONTS; i++ )
	{
		if ( !bFontsLoaded )
			hFonts[i] = surface()->CreateFont();
		bool bSuccess = surface()->SetFontGlyphSet( hFonts[i], "Tahoma", 1 + i, 1, 0, 0,

#ifndef SHADER_EDITOR_DLL_SWARM
			ISurface::FONTFLAG_CUSTOM |
			ISurface::FONTFLAG_DROPSHADOW |
			ISurface::FONTFLAG_ANTIALIAS
#else
			FONTFLAG_CUSTOM |
			FONTFLAG_DROPSHADOW |
			FONTFLAG_ANTIALIAS
#endif

			);
		if ( !bSuccess )
			hFonts[i] = 0;
		/*
		if ( !bFontsLoaded )
			hFonts2[i] = surface()->CreateFont();
		bSuccess = surface()->SetFontGlyphSet( hFonts2[i], "Tahoma", 1 + i, 1, 0, 0, ISurface::FONTFLAG_CUSTOM
			//| ISurface::FONTFLAG_OUTLINE
			//| ISurface::FONTFLAG_ANTIALIAS
			//| ISurface::FONTFLAG_GAUSSIANBLUR
			);
		if ( !bSuccess )
			hFonts2[i] = 0;
		*/
	}
	_Font_Marlett = pScheme->GetFont( "Marlett", false );

	bFontsLoaded = true;
}

void CEditorRoot::ToggleFullScreen()
{
	if ( !pEditorRoot->IsVisible() )
		return;

	m_bHalfView = !m_bHalfView;
	if ( !m_bHalfView )
		m_bHasInput = true;

	if ( pPreview )
		pPreview->RememberPosition();

	InvalidateLayout( true );

	if ( pPreview )
		pPreview->RestorePosition();

	if ( pNodeSheet )
	{
		pNodeSheet->InvalidateLayout();
		pNodeSheet->ScrollToActivePage();
	}
}
void CEditorRoot::ToggleVisible()
{
	SetVisible( !IsVisible() );

	if ( IsVisible() )
		MoveToFront();
}
void CEditorRoot::ToggleInput()
{
	if ( !pEditorRoot->IsVisible() )
		return;

	m_bHasInput = !m_bHasInput;
	if ( !m_bHalfView )
		m_bHasInput = true;

	if ( pPreview )
		pPreview->RememberPosition();
	InvalidateLayout();
	if ( pPreview )
		pPreview->RestorePosition();
}
