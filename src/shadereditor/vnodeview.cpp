#include "cbase.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include "vgui/ISurface.h"
#include "ienginevgui.h"
#include <vgui_controls/button.h>
#include <vgui_controls/textentry.h>
#include <vgui_controls/richtext.h>
#include <vgui_controls/menu.h>
#include <vgui_controls/menuitem.h>
#include <vgui_controls/propertydialog.h>
#include <vgui_controls/propertysheet.h>
#include <vgui_controls/propertypage.h>

#include "materialsystem/imesh.h"
#include "materialsystem/ITexture.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/imaterialsystem.h"

#include "collisionutils.h"
#include "vgui_controls/animationcontroller.h"

#include "editorCommon.h"
#include "vNodeView.h"
#include "vSmartObject.h"
#include "vSmartTooltip.h"
#include "vSheets.h"

//#include "memtrack.h"

bool CNodeView::bRenderingScreenShot = false;
Vector4D CNodeView::vecScreenshotBounds = Vector4D( 0,0,0,0 );

#define VIEWSPEED_PAN editor_movespeed.GetFloat()
#define VIEWSPEED_ZOOM editor_movespeed.GetFloat()

#define BRIDGE_DRAG_TOLERANCE 10

#define VIEWZOOM_SPEED editor_zoomspeed.GetFloat()

#define GRID_CELL_SIZE 100.0f

CNodeView::CNodeView( Panel *parent, CEditorRoot *root, /*FlowGraphType_t type,*/ const char *pElementName ) : BaseClass( parent, pElementName )
{
	m_FlowGraphType = FLOWGRAPH_HLSL;
	InitColors();

	m_bPreviewMode = root != NULL && parent != NULL;
	//vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	//SetScheme( scheme );
	pEditorRoot = root;
	//LoadControlSettings("resource/shadereditor_root.res");
	SetProportional(false);

	m_flZoom = 8;
	m_flZoomGoal = 8;
	m_vecPosition.Init();
	m_vecPositionGoal.Init();

	m_bHistoryDirty = false;
	m_lTime_MouseLastMoved = 0.0f;
	m_flErrorTime = 0;

	SetupVguiTex( m_iTex_Darken, "shadereditor/darken" );

	pJackLast = NULL;
	m_pCurBridge = NULL;
	m_pLastPreviewData = NULL;
	m_pTooltip = NULL;
	m_iLastPreviewFlags = 0;

	m_bDirtySolvers = false;
	//ClearCurrentSolverResource();

	m_ShaderData = NULL;
	m_PPEData = NULL;
	//if ( m_FlowGraphType == FLOWGRAPH_HLSL )
	//	m_ShaderData = new GenericShaderData();

	m_StackIndex = 0;
	m_StackIndex_LastCompiled = 0;
	m_HistoryIndex = 0;
	m_HistoryIndex_LastSaved = 0;
	m_bLastCompileSuccessFull = false;

	pKV_Mat_PsIn = NULL;
	m_pMat_PsIn = NULL;
	m_pPreview_Material = NULL;
	m_pPreview_KV = NULL;
	m_pPrevBackground_Material = NULL;
	m_pPrevBackground_KV = NULL;

	m_szEnvmap[0] = '\0';
	//m_pTexCubemap = NULL;
	m_szShaderName[0] = '\0';
	m_bRenderBackground = false;

	CAutoSolverCallback::Startup();
	AddSolverClient( this );

	m_iCursorLast = dc_user;

	UpdatePsinMaterial(NULL);
	InitPreviewMaterials();

	SetupVguiTex( m_iTex_BusyIco, "shadereditor/busy" );

	Init();
}
CNodeView::~CNodeView()
{
	FlushHistory();

	RemoveSolverClient( this );
	CAutoSolverCallback::Shutdown();
	//ClearCurrentSolverResource();
	PurgeCanvas( true );
	PurgeSolverClients();

	ClearShaderData();
	delete m_pLastPreviewData;
	ClearPPEData();

	if (m_pMat_PsIn)
	{
		pKV_Mat_PsIn->Clear();
		//m_pMat_PsIn->Release();
		//m_pMat_PsIn->DeleteIfUnreferenced();
		//pKV_Mat_PsIn = NULL;
		ForceDeleteMaterial( &m_pMat_PsIn );
	}

	DestroyPreviewMaterials();

	KillDragBridge();
	//if ( pKV_Mat_PsIn )
	//{
	//	delete ((void*)pKV_Mat_PsIn);
	//	pKV_Mat_PsIn = NULL;
	//}

	DestroyTooltip();
}

CNodeView::FlowGraphType_t CNodeView::GetFlowgraphType()
{
	return m_FlowGraphType;
}
void CNodeView::SetFlowgraphType( FlowGraphType_t type, bool bValidateGraphData )
{
	if ( bValidateGraphData )
	{
		if ( type == FLOWGRAPH_HLSL )
			GenerateShaderData();
		else
			ClearShaderData();

		if ( type == FLOWGRAPH_POSTPROC )
			GeneratePPEData();
		else
			ClearPPEData();
	}

	m_FlowGraphType = type;
}

void CNodeView::ClearShaderData()
{
	delete m_ShaderData;
	m_ShaderData = NULL;
}
GenericShaderData &CNodeView::GetDataForModify()
{
	Assert( m_ShaderData && GetFlowgraphType() == FLOWGRAPH_HLSL );
	return *m_ShaderData;
}
void CNodeView::GenerateShaderData()
{
	if ( !m_ShaderData )
		m_ShaderData = new GenericShaderData();
}

void CNodeView::ClearPPEData()
{
	delete m_PPEData;
	m_PPEData = NULL;
}
void CNodeView::GeneratePPEData()
{
	if ( !m_PPEData )
		m_PPEData = new GenericPPEData();
}


void CNodeView::InitPreviewMaterials()
{
	if ( !m_pPreview_Material )
	{
		Assert( !m_pPreview_KV );

		m_pPreview_KV = new KeyValues( "EDITOR_SHADER" );
		m_pPreview_Material = materials->CreateMaterial( "__main_preview", m_pPreview_KV );
		Assert( m_pPreview_Material );

		m_pPreview_Material->Refresh();
	}

	if ( !m_pPrevBackground_Material )
	{
		Assert( !m_pPrevBackground_KV );

		m_pPrevBackground_KV = new KeyValues( "BACKGROUND_PREV" );
		m_pPrevBackground_Material = materials->CreateMaterial( "__bg_preview", m_pPrevBackground_KV );
		Assert( m_pPrevBackground_Material );

		m_pPrevBackground_Material->Refresh();
	}
}

void CNodeView::DestroyPreviewMaterials()
{
	if ( m_pPreview_Material )
	{
		m_pPreview_KV->Clear();
		ForceDeleteMaterial( &m_pPreview_Material );
		m_pPreview_KV = NULL;
	}

	if ( m_pPrevBackground_Material )
	{
		m_pPrevBackground_KV->Clear();
		ForceDeleteMaterial( &m_pPrevBackground_Material );
		m_pPrevBackground_KV = NULL;
	}
}

void CNodeView::SetEnvmap( const char *pszPath )
{
	//m_pTexCubemap = NULL;
	if ( pszPath )
	{
		Q_snprintf( m_szEnvmap, sizeof(m_szEnvmap), "%s", pszPath );
		//m_pTexCubemap = materials->FindTexture( m_szEnvmap, TEXTURE_GROUP_CUBE_MAP, false );

		//if ( IsErrorTexture( m_pTexCubemap ) )
		//	m_pTexCubemap = NULL;
	}
	else
		m_szEnvmap[0] = '\0';
}

const char *CNodeView::GetEnvmap()
{
	if ( Q_strlen(m_szEnvmap) )
		return m_szEnvmap;
	return NULL;
}

//ITexture *CNodeView::GetEnvmapTexture()
//{
//	return m_pTexCubemap;
//}

void CNodeView::SetShadername( const char *pszPath )
{
	if ( pszPath )
		Q_snprintf( m_szShaderName, sizeof(m_szShaderName), "%s", pszPath );
	else
		m_szShaderName[0] = '\0';
}

const char *CNodeView::GetShadername()
{
	if ( Q_strlen(m_szShaderName) )
		return m_szShaderName;
	return NULL;
}

void CNodeView::SetUsingBackgroundMaterial( bool bUse )
{
	m_bRenderBackground = bUse;
}

const bool CNodeView::IsUsingBackgroundMaterial()
{
	return m_bRenderBackground;
}

IMaterial *CNodeView::GetPreviewMeshMaterial()
{
	return m_pPreview_Material;
}

IMaterial *CNodeView::GetPreviewBackgroundMaterial()
{
	return m_pPrevBackground_Material;
}

KeyValues *CNodeView::GetKVMeshMaterial()
{
	return m_pPreview_KV;
}

KeyValues *CNodeView::GetKVBackgroundMaterial()
{
	return m_pPrevBackground_KV;
}

GenericShaderData *CNodeView::GetPreviewData()
{
	return m_pLastPreviewData;
}

void CNodeView::PurgeSolverClients()
{
	m_hSolverClients.Purge();
}
void CNodeView::AddSolverClient( ISolverRequester *client )
{
	if ( m_hSolverClients.IsValidIndex( m_hSolverClients.Find( client ) ) )
		return;
	m_hSolverClients.AddToTail( client );
	MakeSolversDirty();
}
void CNodeView::RemoveSolverClient( ISolverRequester *client )
{
	m_hSolverClients.FindAndRemove( client );
}
void CNodeView::PrepShaderData()
{
	Assert( m_ShaderData );

	CNodeVSInput *pVS_IN = (CNodeVSInput*)GetNodeFromType( HLSLNODE_VS_IN );
	CNodeVSOutput *pVS_OUT = (CNodeVSOutput*)GetNodeFromType( HLSLNODE_VS_OUT );
	CNodePSOutput *pPS_OUT = (CNodePSOutput*)GetNodeFromType( HLSLNODE_PS_OUT );

	const bool bIsPreview = m_ShaderData->shader->bPreviewMode; //IsPreviewModeEnabled();

	//m_ShaderData->shader->bPreviewMode = bIsPreview;
	if ( pVS_IN )
		m_ShaderData->VS_IN = pVS_IN->GetSetup();
	if ( pVS_OUT )
		m_ShaderData->VS_to_PS = pVS_OUT->GetSetup();
	if ( pPS_OUT )
		m_ShaderData->PS_OUT = pPS_OUT->GetSetup();

	if ( bIsPreview )
		DoEnvmapOverride();

	const char *shaderName = GetShadername(); //pEditorRoot->GetCurrentShaderName();
	char *tmpName = new char[ MAX_PATH ];

	if ( shaderName )
	{
		Q_FileBase( shaderName, tmpName, MAX_PATH + 1 );

		Assert( m_ShaderData->shader->Filename == NULL );
		char tmp[MAX_PATH*4];
		if ( g_pFullFileSystem->FullPathToRelativePath( shaderName, tmp, sizeof( tmp ) ) )
		{
			int len = Q_strlen( tmp ) + 1;
			m_ShaderData->shader->Filename = new char[len];
			Q_strcpy( m_ShaderData->shader->Filename, tmp );
		}
		else
			AutoCopyStringPtr( shaderName, &m_ShaderData->shader->Filename );
	}

	if ( !shaderName || Q_strlen( tmpName ) < 1 )
		Q_snprintf( tmpName, MAX_PATH, "UNNAMEDSHADER" );

	m_ShaderData->name = tmpName;
	m_ShaderData->iStackIdx = GetStackIndex();
}
void CNodeView::CleanupShaderData()
{
	Assert( m_ShaderData );

	delete [] m_ShaderData->name;
	m_ShaderData->name = NULL;

	delete [] m_ShaderData->shader->Filename;
	m_ShaderData->shader->Filename = NULL;
}
void CNodeView::CleanupNodes()
{
	CUtlVector< CHLSL_Var* >hVarGarbage;
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *pN = m_hNodeList[ i ];

		for ( int j = 0; j < pN->GetNumJacks_Out(); j++ )
		{
			CJack *pJ = pN->GetJack_Out( j );
			CHLSL_Var *var = pJ->GetTemporaryVarTarget();

			if ( !var )
				continue;

			if ( !hVarGarbage.HasElement( var ) )
				hVarGarbage.AddToTail( var );
		}
		for ( int j = 0; j < pN->GetNumJacks_In(); j++ )
		{
			CJack *pJ = pN->GetJack_In( j );
			CHLSL_Var *var = pJ->GetTemporaryVarTarget();

			if ( !var )
				continue;

			if ( !hVarGarbage.HasElement( var ) )
				hVarGarbage.AddToTail( var );
		}
	}

	hVarGarbage.PurgeAndDeleteElements();

	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *n = m_hNodeList[ i ];
		//n->RemoveSolvers();
		n->SweepJackHlslCache();
	}
}

void CNodeView::SetPreviewMode( bool bEnabled )
{
	m_bPreviewMode = bEnabled;
}

bool CNodeView::IsPreviewModeEnabled()
{
	return m_bPreviewMode;
}

void CNodeView::UpdateSolverClients()
{
	Assert( m_ShaderData );

	CNodeVSOutput *pVS_OUT = (CNodeVSOutput*)GetNodeFromType( HLSLNODE_VS_OUT );
	CNodePSOutput *pPS_OUT = (CNodePSOutput*)GetNodeFromType( HLSLNODE_PS_OUT );
	const bool bVertexShaderReady = pVS_OUT && pVS_OUT->GetErrorLevel() == ERRORLEVEL_NONE;
	const bool bPixelShaderReady = pPS_OUT && pPS_OUT->GetErrorLevel() == ERRORLEVEL_NONE;

	PrepShaderData();

#if DEBUG
	for ( int i = 0; i < hErrorList.Count(); i++ )
		AllocCheck_FreeS("LimitReport_t");
#endif
	hErrorList.PurgeAndDeleteElements();

	if (
		!ReportErrors( m_ShaderData, hErrorList ) && 
		( !m_ShaderData->IsPreview() || pEditorRoot->ShouldAutoCompile() )
		)
	{
		for ( int i = 0; i < m_hSolverClients.Count(); i++ )
		{
			ISolverRequester *c = m_hSolverClients[ i ];
			c->OnSolverUpdated( this, m_ShaderData, bVertexShaderReady, bPixelShaderReady );
		}
	}

	CleanupShaderData();
}
void CNodeView::UpdatePPECache()
{
	if ( m_hSolverStack_PostProcessing.Count() < 1 )
		return;

	const char *pszShaderName = GetShadername();
	if ( !pszShaderName )
		return;

	char tmp[MAX_PATH*4];
	Q_FileBase( pszShaderName, tmp, sizeof(tmp) );

	int index = GetPPCache()->FindPostProcessingEffect( tmp );

	if ( index < 0 )
		return;

	EditorPostProcessingEffect *fx = GetPPCache()->GetPostProcessingEffect( index );
	Assert( fx );
	fx->ReplacePostProcessingChain( m_hSolverStack_PostProcessing );
	fx->bIsEnabled = true;

	DestroySolverStack( m_hSolverStack_PostProcessing );
}
void CNodeView::OnSolvingFinsihed()
{
	if ( ShouldCallCompiler() )
	{
		UpdateSolverClients();

		if ( IsPreviewModeEnabled() && pEditorRoot->ShouldAutoFullcompile() )
		{
			SetPreviewMode( false );
			UpdateSolverClients();
			SetPreviewMode( true );
		}
	}
	else if ( GetFlowgraphType() == FLOWGRAPH_POSTPROC )
		UpdatePPECache();
}
void CNodeView::OnCompilationEnd( int flags, int exitcode, GenericShaderData *data )
{
	//Assert( GetFlowgraphType() == FLOWGRAPH_HLSL );

	const bool bError = exitcode != 0;
	m_StackIndex_LastCompiled = data->iStackIdx;
	m_bLastCompileSuccessFull = !bError;

	//if ( pEditorRoot != NULL )
	//{
	//	pEditorRoot->CompileCallback( bError, data->iStackIdx );
	//}

	//if ( !data->bPreview && !bError )
	//	df_SaveDump_File( data->name, *data->shader );

	//CopyShaders( flags, data );

	if ( bError )
	{
		m_flErrorTime = 1;
		return;
	}

	if ( data->IsPreview() )
	{
		CAutoSolverCallback::CopyShaders( flags, data );

		delete m_pLastPreviewData;
		m_pLastPreviewData = NULL;

		//if ( !bError )
		{
			m_iLastPreviewFlags = flags;
			m_pLastPreviewData = new GenericShaderData( *data );
		}
		//else
		//	m_iLastPreviewFlags = 0;

		UploadPreviewData();
	}
	else
	{
		//if ( flags & ACTIVEFLAG_VS && flags & ACTIVEFLAG_PS )
		//	pEditorRoot->SendFullyCompiledShader( data );
	}

	//delete data;
}

void CNodeView::UploadPreviewData()
{
	if ( pEditorRoot->GetSafeFlowgraph() != this )
		return;

	if ( !m_iLastPreviewFlags || !m_pLastPreviewData )
	{
		Update3DPrevMaterial( NULL );
		Update3DPrevMaterial( NULL, 1 );
		return;
	}

	if ( m_iLastPreviewFlags & ACTIVEFLAG_VS_POS )
	{
		char _vs_pos[MAX_PATH];
		ComposeShaderName_Compiled( m_pLastPreviewData, false, false, _vs_pos, MAX_PATH, true );
		UpdatePsinMaterial( _vs_pos, m_pLastPreviewData );
	}
	else
		UpdatePsinMaterial( NULL );

	if ( m_iLastPreviewFlags & ACTIVEFLAG_VS && m_iLastPreviewFlags & ACTIVEFLAG_PS )
		Update3DPrevMaterial( m_pLastPreviewData );
	if ( m_iLastPreviewFlags & ACTIVEFLAG_VS )
		Update3DPrevMaterial( m_pLastPreviewData, 1 );
}

void CNodeView::UpdatePsinMaterial( const char *vsName, GenericShaderData *data )
{
	if ( !m_pMat_PsIn )
	{
		Assert( !pKV_Mat_PsIn );
		/*KeyValues **/ pKV_Mat_PsIn = new KeyValues( "NODE_PSIN" );
		//pKV_Mat_PsIn = new KeyValues( "NODE_PSIN" ); 
//		::gProcShaderCTRL->SendTempPsInputShaderName( "psin_vs20" );
		m_pMat_PsIn = materials->CreateMaterial( "__npv_psin", pKV_Mat_PsIn );
		//pKV_Mat_PsIn->Clear();

		pKV_Mat_PsIn->SetString( "$VSNAME", "psin_vs20" );
		m_pMat_PsIn->Refresh();
		//SetMaterialVar_String( m_pMat_PsIn, "$VSNAME", "psin_vs20" );
		//m_pMat_PsIn->Refresh();
		//IMaterialVar *VS = m_pMat_PsIn->FindVar( "$VSNAME", &bFound2 );
		//VS->SetStringValue( "psin_vs20" );
		UpdatePsinMaterialVar();
		Assert(m_pMat_PsIn);
		return;
	}
	//IMaterialVar *pVSName = m_pMat_PsIn->FindVar( "$VSNAME", &bFound );
	//Assert(bFound);
	//Assert(pKV_Mat_PsIn);
	Assert(m_pMat_PsIn);
	Assert( !IsErrorMaterial( m_pMat_PsIn ) );

	const char *pDefault = "psin_vs20";
	const char *szTmp = pDefault;
	if ( vsName && Q_strlen( vsName ) )
		szTmp = vsName;

	//SetMaterialVar_String( m_pMat_PsIn, "$VSNAME", szTmp );
	pKV_Mat_PsIn->SetString( "$VSNAME", szTmp );

	if ( data )
	{
		//SetMaterialVar_Int( m_pMat_PsIn, "$VFMT_FLAGS", data->shader->iVFMT_flags );
		//SetMaterialVar_Int( m_pMat_PsIn, "$VFMT_NUMTEXCOORDS", data->shader->iVFMT_numTexcoords );
		//SetMaterialVar_Int( m_pMat_PsIn, "$VFMT_USERDATA", data->shader->iVFMT_numUserData );
		pKV_Mat_PsIn->SetInt( "$VFMT_FLAGS", data->shader->iVFMT_flags );
		pKV_Mat_PsIn->SetInt( "$VFMT_NUMTEXCOORDS", data->shader->iVFMT_numTexcoords );
		pKV_Mat_PsIn->SetInt( "$VFMT_USERDATA", data->shader->iVFMT_numUserData );
		pKV_Mat_PsIn->SetInt( "$VFMT_TEXCOORDDIM_0", data->shader->iVFMT_texDim[0] );
		pKV_Mat_PsIn->SetInt( "$VFMT_TEXCOORDDIM_1", data->shader->iVFMT_texDim[1] );
		pKV_Mat_PsIn->SetInt( "$VFMT_TEXCOORDDIM_2", data->shader->iVFMT_texDim[2] );
	}

	m_pMat_PsIn->Refresh();
	m_pMat_PsIn->RecomputeStateSnapshots();
	UpdatePsinMaterialVar();
}

void CNodeView::UpdatePsinMaterialVar()
{
	bool bFound = false;
	m_pMatVar_PsIn_Operation = m_pMat_PsIn->FindVar( "$OPERATION", &bFound );
	Assert(bFound);
}

void CNodeView::OnContainerRemoved( CBaseContainerNode *container )
{
	int idx = m_hContainerNodes.Find( container );
	if ( !m_hContainerNodes.IsValidIndex(idx) )
		return;
	m_hContainerNodes.Remove( idx );

	Assert( !m_hContainerNodes.IsValidIndex( m_hContainerNodes.Find( container ) ) );
}
void CNodeView::OnContainerAdded( CBaseContainerNode *container )
{
	Assert( !m_hContainerNodes.IsValidIndex( m_hContainerNodes.Find( container ) ) );
	m_hContainerNodes.AddToTail( container );
}
void CNodeView::ListContainerAtPos( Vector2D &pos, CUtlVector< CBaseContainerNode* > &hList )
{
	hList.Purge();
	for ( int i = 0; i < m_hContainerNodes.Count(); i++ )
	{
		if ( m_hContainerNodes[i]->IsInContainerBounds( pos ) )
			hList.AddToTail( m_hContainerNodes[i] );
	}
}

void CNodeView::PurgeCanvas( bool OnShutdown )
{
	if ( m_pCurBridge )
	{
		delete m_pCurBridge;
		m_pCurBridge = NULL;
	}
	DestroyAllSolvers();
	SaveDeleteVector( m_hBridgeList );
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
		m_hNodeList[ i ]->MarkForDeletion();
	SaveDeleteVector( m_hNodeList );
	m_hNodesInMove.Purge();
	m_hTmpNodeIndex.Purge();

	m_hContainerNodes.Purge();
	if ( !OnShutdown )
		UpdatePsinMaterial( NULL );

#if DEBUG
	for ( int i = 0; i < hErrorList.Count(); i++ )
		AllocCheck_FreeS("LimitReport_t");
#endif
	hErrorList.PurgeAndDeleteElements();
}

void CNodeView::InitCanvas( FlowGraphType_t type, GenericShaderData *newData )
{
	PurgeCanvas();

	SetFlowgraphType( type );

	if ( type == FLOWGRAPH_HLSL )
	{
		CBaseNode *n = SpawnNode( HLSLNODE_VS_IN );
		n->SetPosition( Vector2D( -CRUCIALNODE_CANVAS_BASE_X, CRUCIALNODE_CANVAS_BASE_Y + CRUCIALNODE_CANVAS_OFFSET_Y ) );

		n = SpawnNode( HLSLNODE_VS_OUT );
		n->SetPosition( Vector2D( CRUCIALNODE_CANVAS_BASE_X - CRUCIALNODE_MIN_SIZE_X, CRUCIALNODE_CANVAS_BASE_Y + CRUCIALNODE_CANVAS_OFFSET_Y ) );

		n = SpawnNode( HLSLNODE_PS_IN );
		n->SetPosition( Vector2D( -CRUCIALNODE_CANVAS_BASE_X, -CRUCIALNODE_CANVAS_BASE_Y + CRUCIALNODE_CANVAS_OFFSET_Y ) );

		n = SpawnNode( HLSLNODE_PS_OUT );
		n->SetPosition( Vector2D( CRUCIALNODE_CANVAS_BASE_X - CRUCIALNODE_MIN_SIZE_X, -CRUCIALNODE_CANVAS_BASE_Y + CRUCIALNODE_CANVAS_OFFSET_Y ) );

		if ( newData )
		{
			ClearShaderData();

			Assert( !m_ShaderData );
			m_ShaderData = new GenericShaderData( *newData );
		}

		Assert( m_ShaderData );
	}
	else if ( type == FLOWGRAPH_POSTPROC )
	{
		CBaseNode *n = SpawnNode( HLSLNODE_POSTPROCESSING_INPUT );
		n->SetPosition( Vector2D( -CRUCIALNODE_CANVAS_BASE_X, CRUCIALNODE_CANVAS_OFFSET_Y / 2 ) );

		n = SpawnNode( HLSLNODE_POSTPROCESSING_OUTPUT );
		n->SetPosition( Vector2D( CRUCIALNODE_CANVAS_BASE_X - CRUCIALNODE_MIN_SIZE_X, CRUCIALNODE_CANVAS_OFFSET_Y / 2 ) );
	}
}
void CNodeView::Init(void)
{
	ResetView();
	PurgeCanvas();
}

void CNodeView::OnBridgeRemoved( CBridge *b )
{
	if ( b == m_pCurBridge )
	{
		m_pCurBridge = NULL;
	}
	m_hBridgeList.FindAndRemove( b );

	OnHierachyChanged();
}
void CNodeView::OnJackRemoved( const CJack *j )
{
	if ( pJackLast == j )
	{
		pJackLast = NULL;
	}

	OnHierachyChanged();
}

void CNodeView::UpdateRTs()
{
	if ( CNodeView::bRenderingScreenShot )
		return;

	if ( GetFlowgraphType() != FLOWGRAPH_HLSL )
		return;

	CViewSetup setup;
	setup.origin = Vector( 0, 0, 0.5f );
	setup.angles = QAngle( 90, 0, 0 );
	setup.m_bOrtho = true;
	setup.x = 0;
	setup.y = 0;
	setup.width = GetRTSize();
	setup.height = GetRTSize();
	setup.m_OrthoTop = -0.5f;
	setup.m_OrthoLeft = -0.5f;
	setup.m_OrthoBottom = 0.5f;
	setup.m_OrthoRight = 0.5f;
	setup.zNear = 0;
	setup.zFar = 1.0f;

	ResetTileInfo();

	int numSolvers_P = m_hSolverStack_PixelShader.Count();
	int numSolvers_U = m_hSolverStack_NoHierachy.Count();

	CMatRenderContextPtr pRenderContext( materials );

	CalcPuzzleSize( numSolvers_P + numSolvers_U );

	pRenderContext->ClearColor4ub( 0, 0, 0, 255 );
	pRenderContext->PushRenderTargetAndViewport( GetPingPongRT(0) );
	pRenderContext->ClearBuffers( true, false );
#ifndef SHADER_EDITOR_DLL_2006
	pRenderContext.SafeRelease();
#endif

	Frustum tmp;
#ifdef SHADER_EDITOR_DLL_2006
	render->Push3DView( setup, 0, true, GetPingPongRT( 0 ), tmp );
#else
	render->Push3DView( setup, 0, GetPingPongRT( 0 ), tmp );
#endif
	//pRenderContext->PushRenderTargetAndViewport( GetPingPongRT( 0 ) );

	gShaderEditorSystem->UpdateConstants( &setup );
	if ( numSolvers_P )
		RenderSolvers( m_hSolverStack_PixelShader );
	if ( numSolvers_U )
		RenderSolvers( m_hSolverStack_NoHierachy );
	gShaderEditorSystem->UpdateConstants();

#ifndef SHADER_EDITOR_DLL_2006
	pRenderContext.GetFrom( materials );
#endif
	pRenderContext->PopRenderTargetAndViewport();
	render->PopView( tmp );
}
void CNodeView::RenderSolvers( CUtlVector< CHLSL_SolverBase* > &hSolvers )
{
	CMatRenderContextPtr pRenderContext( materials );

	Preview2DContext context;
	context.pRenderContext = pRenderContext;

	context.pMat_PsIn = m_pMat_PsIn;
	context.pMat_PsInOP = m_pMatVar_PsIn_Operation;

	ResetVariableIndices( hSolvers );
	int x,y,w,t;
	for ( int i = 0; i < hSolvers.Count(); i++ )
	{
		if ( !hSolvers[i]->IsRenderable() )
			continue;
		GetCurPuzzleView( x, y, w, t );
		pRenderContext->Viewport( x, y, w, t );

		hSolvers[ i ]->Invoke_Render( context );
		pRenderContext->CopyRenderTargetToTexture( GetPingPongRT(1) );
		OnPuzzleDrawn();
	}
}
void CNodeView::RenderSingleSolver( Preview2DContext &c, IMaterial *pMat )
{
	CMeshBuilder pMeshBuilder;
	IMesh *pMesh = c.pRenderContext->GetDynamicMesh( true, 0, 0, pMat );
	pMeshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );
#ifndef SHADER_EDITOR_DLL_2006
	pMeshBuilder.SetCompressionType( VERTEX_COMPRESSION_NONE );
#endif

	float norm[3] = { 0, 0, 1 };
	float tan_s[3] = { 0, -1, 0 };
	float tan_t[3] = { 1, 0, 0 };
	float tan_s_u[4] = { 0, -1, 0, 1 };

	pMeshBuilder.Position3f( 0.5f, 0.5f, 0 );
	pMeshBuilder.Normal3fv( norm );
	pMeshBuilder.TangentS3fv( tan_s );
	pMeshBuilder.TangentT3fv( tan_t );
	pMeshBuilder.UserData( tan_s_u );
	for ( int i = 0; i < 3; i++ )
		pMeshBuilder.TexCoord2f( i, 0, 0 );
	pMeshBuilder.Color4f( 1, 1, 1, 1 );
	pMeshBuilder.AdvanceVertex();

	pMeshBuilder.Position3f( 0.5f, -0.5f, 0 );
	pMeshBuilder.Normal3fv( norm );
	pMeshBuilder.TangentS3fv( tan_s );
	pMeshBuilder.TangentT3fv( tan_t );
	pMeshBuilder.UserData( tan_s_u );
	for ( int i = 0; i < 3; i++ )
		pMeshBuilder.TexCoord2f( i, 1, 0 );
	pMeshBuilder.Color4f( 1, 1, 1, 1 );
	pMeshBuilder.AdvanceVertex();

	pMeshBuilder.Position3f( -0.5f, -0.5f, 0 );
	pMeshBuilder.Normal3fv( norm );
	pMeshBuilder.TangentS3fv( tan_s );
	pMeshBuilder.TangentT3fv( tan_t );
	pMeshBuilder.UserData( tan_s_u );
	for ( int i = 0; i < 3; i++ )
		pMeshBuilder.TexCoord2f( i, 1, 1 );
	pMeshBuilder.Color4f( 1, 1, 1, 1 );
	pMeshBuilder.AdvanceVertex();

	pMeshBuilder.Position3f( -0.5f, 0.5f, 0 );
	pMeshBuilder.Normal3fv( norm );
	pMeshBuilder.TangentS3fv( tan_s );
	pMeshBuilder.TangentT3fv( tan_t );
	pMeshBuilder.UserData( tan_s_u );
	for ( int i = 0; i < 3; i++ )
		pMeshBuilder.TexCoord2f( i, 0, 1 );
	pMeshBuilder.Color4f( 1, 1, 1, 1 );
	pMeshBuilder.AdvanceVertex();

	pMeshBuilder.End();
	pMesh->Draw();
}

void CNodeView::MakeSolversDirty()
{
	m_bDirtySolvers = true;
	m_StackIndex++;
}
void CNodeView::OnHierachyChanged()
{
	MakeSolversDirty();
}
void CNodeView::DoFullHierachyUpdate()
{
	CUtlVector< CBaseNode* > m_hList_Bottom;
	CUtlVector< CBaseNode* > m_hList_Top;
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *n = m_hNodeList[ i ];
		n->Recursive_AddTailNodes( m_hList_Bottom, false );
	}
	for ( int i = 0; i < m_hList_Bottom.Count(); i++ )
	{
		CBaseNode *n = m_hList_Bottom[ i ];
		n->Recursive_AddTailNodes( m_hList_Top, true );
	}
	for ( int i = 0; i < m_hList_Top.Count(); i++ )
		m_hList_Top[i]->OnUpdateHierachy( NULL, NULL );
	m_hList_Top.Purge();
	m_hList_Bottom.Purge();
}
void CNodeView::ResetTempHierachy()
{
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
		m_hNodeList[ i ]->SetTempHierachyType( HLSLHIERACHY_PENDING );
}
bool CNodeView::ShouldCallCompiler()
{
	if ( m_ShaderData == NULL )
		return false;

	if ( GetFlowgraphType() == FLOWGRAPH_HLSL )
		return true;

	return false;
}

void CNodeView::UpdateUserErrors()
{
	const FlowGraphType_t type = GetFlowgraphType();
	CUtlVector< int > hActiveErrors;
	CUtlVector< char* > hErrorParams;

	if ( type == FLOWGRAPH_HLSL )
	{
		Assert( m_ShaderData != NULL );

		bool bShowTextureMismatchError = false;
		char paramName[MAX_PATH];
		paramName[0] = '\0';
		//const CUtlVector< SimpleTexture* > &hTextures = m_ShaderData->shader->pPS_Identifiers->hList_Textures;
		CUtlVector< SimpleTexture* > hTextures;
		hTextures.AddVectorToTail( m_ShaderData->shader->pPS_Identifiers->hList_Textures );
		hTextures.AddVectorToTail( m_ShaderData->shader->pVS_Identifiers->hList_Textures );

		for ( int i = 0; i < hTextures.Count() && !bShowTextureMismatchError; i++ )
		{
			const int baseMode = hTextures[i]->iTextureMode;

			if ( !IS_TEXTURE_SAMPLER_USING_CUSTOM_TEXTURE( baseMode ) )
				continue;

			for ( int t = 0; t < hTextures.Count() && !bShowTextureMismatchError; t++ )
			{
				if ( t == i )
					continue;

				const int childMode = hTextures[t]->iTextureMode;

				if ( baseMode != childMode )
					continue;

				const bool bCustomParam = baseMode == HLSLTEX_CUSTOMPARAM;

				if ( bCustomParam )
				{
					Assert( hTextures[i]->szParamName && hTextures[t]->szParamName );

					if ( hTextures[i]->szParamName && hTextures[t]->szParamName &&
						Q_stricmp( hTextures[i]->szParamName, hTextures[t]->szParamName ) )
						continue;
				}

				const bool bHasTex_fb_0 = hTextures[i]->szFallbackName != NULL;
				const bool bHasTex_fb_1 = hTextures[t]->szFallbackName != NULL;
				const bool bHasTex_prev_0 = hTextures[i]->szTextureName != NULL;
				const bool bHasTex_prev_1 = hTextures[t]->szTextureName != NULL;

				if ( bHasTex_fb_0 != bHasTex_fb_1 || bHasTex_prev_0 != bHasTex_prev_1 )
					bShowTextureMismatchError = true;
				else if ( bHasTex_fb_0 && Q_stricmp( hTextures[i]->szFallbackName, hTextures[t]->szFallbackName ) ||
						bHasTex_prev_0 && Q_stricmp( hTextures[i]->szTextureName, hTextures[t]->szTextureName ) )
					bShowTextureMismatchError = true;

				if ( bShowTextureMismatchError )
				{
					if ( bCustomParam )
					{
						if ( hTextures[i]->szParamName != NULL )
							Q_snprintf( paramName, sizeof(paramName), "%s", hTextures[i]->szParamName );
						else
							Q_snprintf( paramName, sizeof(paramName), "unnamed custom param" );
					}
					else
						Q_snprintf( paramName, sizeof(paramName), "%s", GetTextureTypeName( baseMode ) );
				}
			}
		}

		if ( bShowTextureMismatchError )
		{
			int len = Q_strlen( paramName ) + 1;
			char *pErrorParam = (len > 1) ? new char[len] : NULL;
			if ( pErrorParam != NULL )
				Q_strcpy( pErrorParam, paramName );

			hActiveErrors.AddToTail( CNodeViewError::NVERROR_TEXTURE_SAMPLER_PARAM_MISMATCH );
			hErrorParams.AddToTail( pErrorParam );
		}
	}
	else if ( type == FLOWGRAPH_POSTPROC )
	{
		CNodePP_Base *pNodeStart = (CNodePP_Base*)GetNodeFromType( HLSLNODE_POSTPROCESSING_INPUT );
		if ( pNodeStart )
		{
			bool bGotNonScene = false;
			bool bGotError = false;
			for ( CNodePP_Base *pNext = pNodeStart->GetNextPPNode(); pNext && !bGotError; pNext = pNext->GetNextPPNode() )
			{
				const bool bScene = pNext->IsSceneNode();
				if ( bGotNonScene && bScene )
					bGotError = true;
				else if ( !bScene )
					bGotNonScene = true;
			}

			if ( bGotError )
			{
				hActiveErrors.AddToTail( CNodeViewError::NVERROR_RENDER_VIEW_SCENE_NOT_AT_START );
				hErrorParams.AddToTail( NULL );
			}
		}
	}

#ifdef SHADER_EDITOR_DLL_2006
	char nodeName[MAX_PATH];
	bool bShowUnsupportedError = false;

	const int unsupportedList[] = {
		HLSLNODE_STUDIO_VERTEXLIGHTING,
		HLSLNODE_STUDIO_PIXELSHADER_LIGHTING,
		HLSLNODE_STUDIO_PIXELSHADER_SPECLIGHTING,
		HLSLNODE_STUDIO_SKINNING,
		HLSLNODE_STUDIO_MORPH,

		HLSLNODE_CONSTANT_LIGHTSCALE,

		HLSLNODE_MATRIX_FLASHLIGHT,
		HLSLNODE_UTILITY_FLASHLIGHT,
		HLSLNODE_CONSTANT_FLASHLIGHTPOS,

		HLSLNODE_STUDIO_VCOMPRESSION,

		HLSLNODE_POSTPROCESSING_RENDER_VIEW,
	};
	const int unsupportedList_size = ARRAYSIZE( unsupportedList );

	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *n = m_hNodeList[i];
		int t = n->GetNodeType();

		for ( int list = 0; list < unsupportedList_size; list++ )
		{
			if ( unsupportedList[list] == t )
			{
				bShowUnsupportedError = true;
				Q_snprintf( nodeName, sizeof( nodeName ), "%s", n->GetName() );
			}
		}

	}

	if ( bShowUnsupportedError )
	{
		char *unsupportedName = NULL;
		AutoCopyStringPtr( nodeName, &unsupportedName );

		hActiveErrors.AddToTail( CNodeViewError::NVERROR_UNSUPPORTED_BY_2006 );
		hErrorParams.AddToTail( unsupportedName );
	}
#endif

	Assert( hErrorParams.Count() == hActiveErrors.Count() );

	// close all messages that are not triggered anymore
	for ( int a = 0; a < m_hUserErrorLabels.Count(); a++ )
	{
		if ( m_hUserErrorLabels[a]->IsClosing() )
			continue;

		const int labelErrorType = m_hUserErrorLabels[a]->GetErrorType();

		if ( !hActiveErrors.HasElement( labelErrorType ) )
			m_hUserErrorLabels[a]->DoClose();
	}

	// remove all errors that are existing already
	for ( int i = 0; i < hActiveErrors.Count(); i++ )
	{
		const int requestedError = hActiveErrors[i];
		for ( int a = 0; a < m_hUserErrorLabels.Count(); a++ )
		{
			const int labelErrorType = m_hUserErrorLabels[a]->GetErrorType();
			if ( labelErrorType == requestedError )
			{
				m_hUserErrorLabels[a]->SetText( hErrorParams[i] );
				if ( m_hUserErrorLabels[a]->IsClosing() )
					m_hUserErrorLabels[a]->DoOpen();

				hActiveErrors.Remove(i);
				delete [] hErrorParams[i];
				hErrorParams.Remove(i);
				i--;
				break;
			}
		}
	}

	// finally, create new missing errors
	for ( int i = 0; i < hActiveErrors.Count(); i++ )
	{
		const int requestedError = hActiveErrors[i];

		CNodeViewError *pError = new CNodeViewError( this, requestedError ); //, hErrorParams[i] );
		pError->SetText( hErrorParams[i] );
		m_hUserErrorLabels.AddToTail( pError );
	}

	for (int i = 0; i < hErrorParams.Count(); i++ )
		delete [] hErrorParams[i];
	hErrorParams.Purge();
}

const int CNodeView::GetNumUserErrorLabels()
{
	return m_hUserErrorLabels.Count();
}

CNodeViewError *CNodeView::GetUserErrorLabel( int index )
{
	return m_hUserErrorLabels[index];
}

void CNodeView::OnDestroyUserError( KeyValues *pKV )
{
	CNodeViewError *p = dynamic_cast<CNodeViewError*>( ((Panel*)pKV->GetPtr( "panel" )) );

	if ( !p || !m_hUserErrorLabels.HasElement(p) )
	{
		Assert( 0 );
		return;
	}

	m_hUserErrorLabels.FindAndRemove( p );

	p->MarkForDeletion();
}

void CNodeView::UpdateUserErrorLayouts()
{
	bool bGotAnimating = false;

	for ( int i = m_hUserErrorLabels.Count() - 1; i >= 0; i-- )
	{
		bGotAnimating = bGotAnimating || m_hUserErrorLabels[i]->IsAnimating();

		if ( bGotAnimating )
			m_hUserErrorLabels[i]->InvalidateLayout( true );
	}
}

void CNodeView::Think_UpdateSolvers()
{
	UpdateSolvers();
}

bool CNodeView::UpdateSolvers()
{
	if ( !m_bDirtySolvers ) //|| m_hNodeList.Count() < 1 )
		return false;

	for ( int i = 0; i < m_hNodeList.Count(); i++ )
		m_hNodeList[ i ]->PreSolverUpdated();

	for ( int i = 0; i < m_hContainerNodes.Count(); i++ )
	{
		m_hContainerNodes[ i ]->FullHierachyUpdate();
	}

	const bool bUsesShaderData = ShouldCallCompiler();

	if ( bUsesShaderData )
	{
		m_ShaderData->shader->iVFMT_texDim[0] = 2;
		m_ShaderData->shader->iVFMT_texDim[1] = 2;
		m_ShaderData->shader->iVFMT_texDim[2] = 2;

		ClearIdentifiers( *m_ShaderData );
	}

#if PSIN_ERROR_WHEN_VSOUT_ERROR
	CBaseNode *pPS_In = GetNodeFromType( HLSLNODE_PS_IN );
	if ( pPS_In )
	{
		pPS_In->SetErrorLevel( pPS_In->PerNodeErrorLevel() );
		pPS_In->JackHierachyUpdate_Out();
	}
#endif

	const FlowGraphType_t type = GetFlowgraphType();

	struct solverInstruction
	{
	public:
		solverInstruction( int iHierachy, CUtlVector< CHLSL_SolverBase* > *phSolver_Output )
		{
			this->iHierachy = iHierachy;
			this->phSolver_Output = phSolver_Output;
		};
		~solverInstruction()
		{
			// explicit, although it's unnecessary
			hList_Source.Purge();
			hList_Solved.Purge();
			hList_Bottom.Purge();
		};

		int iHierachy;
		CUtlVector< CHLSL_SolverBase* > *phSolver_Output;

		CUtlVector< CBaseNode* > hList_Source;
		CUtlVector< CBaseNode* > hList_Solved;
		CUtlVector< CBaseNode* > hList_Bottom;
	};

	CUtlVector< solverInstruction* >hSolverCmds;

	if ( type == FLOWGRAPH_HLSL )
	{
		hSolverCmds.AddToTail( new solverInstruction( HLSLHIERACHY_VS, &m_hSolverStack_VertexShader ) );
		hSolverCmds.AddToTail( new solverInstruction( HLSLHIERACHY_PS, &m_hSolverStack_PixelShader ) );
	}
	else if ( type == FLOWGRAPH_POSTPROC )
	{
		CBaseNode *pPPEOut = GetNodeFromType( HLSLNODE_POSTPROCESSING_OUTPUT );

		if ( pPPEOut && pPPEOut->GetErrorLevel() == ERRORLEVEL_NONE )
			hSolverCmds.AddToTail( new solverInstruction( HLSLHIERACHY_POST_PROCESS, &m_hSolverStack_PostProcessing ) );
	}

	// 'no hierachy' must be last.
	hSolverCmds.AddToTail( new solverInstruction( HLSLHIERACHY_NONE, &m_hSolverStack_NoHierachy ) );

#if DO_SOLVING
	DestroyAllSolvers();
#endif

	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *n = m_hNodeList[ i ];
		n->RemoveSolvers();
		n->SweepJackHlslCache();

		if ( n->GetAsContainer() )
			continue;

		int tmpHierachy = n->GetTempHierachyType();
		if ( tmpHierachy == HLSLHIERACHY_PENDING )
		{
			tmpHierachy = n->GetHierachyTypeIterateFullyRecursive();
			n->SetTempHierachyTypeFullyRecursive( tmpHierachy );
		}

		for ( int s = 0; s < hSolverCmds.Count(); s++ )
		{
			solverInstruction *pCmd = hSolverCmds[s];
			if ( tmpHierachy & pCmd->iHierachy ||
				s == hSolverCmds.Count() - 1 )
			{
				pCmd->hList_Source.AddToTail( n );
				break;
			}
		}
	}

	ResetTempHierachy();

	CUtlVector< CBaseContainerNode* >m_hContainerStack;

#if DO_SOLVING
	for ( int s = 0; s < hSolverCmds.Count(); s++ )
	{
		solverInstruction *pCmd = hSolverCmds[s];

		//CUtlVector< CBaseNode* > m_hList_VertexShader_Bottom;
		for ( int i = 0; i < pCmd->hList_Source.Count(); i++ )
		{
			CBaseNode *n = pCmd->hList_Source[ i ];
			n->Recursive_AddTailNodes( pCmd->hList_Bottom, false, false, NULL, true );
		}
		InvokeCreateSolvers( pCmd->hList_Bottom, pCmd->hList_Solved, m_hContainerStack, m_ShaderData );
		Assert( !m_hContainerStack.Count() );
		m_hContainerStack.Purge();

		Assert( pCmd->phSolver_Output );

		CopySolvers( pCmd->hList_Solved, *pCmd->phSolver_Output );
		ResetVariables( *pCmd->phSolver_Output );
	}
#endif

	CleanupNodes();

	//SetupForRendering( m_hSolverStack_VertexShader );
#if DO_SOLVING
	SetupForRendering( m_hSolverStack_PixelShader );
	SetupForRendering( m_hSolverStack_NoHierachy );
#endif

	if ( bUsesShaderData )
	{
		AllocateIdentifiers( *m_ShaderData->shader->pVS_Identifiers, m_hSolverStack_VertexShader );
		AllocateIdentifiers( *m_ShaderData->shader->pPS_Identifiers, m_hSolverStack_PixelShader );
		AllocateIdentifiers( *m_ShaderData->pUNDEF_Identifiers, m_hSolverStack_NoHierachy );
	}

	UpdateUserErrors();

	if ( bUsesShaderData )
	{
		UniquifyIdentifiers( false, *m_ShaderData->shader->pVS_Identifiers );
		UniquifyIdentifiers( true, *m_ShaderData->shader->pPS_Identifiers );
		UniquifyIdentifiers( true, *m_ShaderData->pUNDEF_Identifiers );

		BuildFormatFlags( *m_ShaderData->shader, m_hSolverStack_VertexShader, m_hSolverStack_PixelShader );

		const bool bIsPreview = IsPreviewModeEnabled();
		m_ShaderData->shader->bPreviewMode = bIsPreview;

		CalcNumDynamicCombos( *m_ShaderData->shader->pVS_Identifiers, bIsPreview );
		CalcNumDynamicCombos( *m_ShaderData->shader->pPS_Identifiers, bIsPreview );
		CalcNumDynamicCombos( *m_ShaderData->pUNDEF_Identifiers, bIsPreview );
	}

#if SHOW_SEDIT_ERRORS
	if ( bUsesShaderData )
	{
		bool bHasLookupSolver = false;
		bool bHasTextureIdentifier = !!m_ShaderData->shader->pPS_Identifiers->hList_Textures.Count();
		for ( int i = 0; i < m_hSolverStack_PixelShader.Count(); i++ )
		{
			if ( dynamic_cast< CHLSL_Solver_TextureSample* >( m_hSolverStack_PixelShader[i] ) )
				bHasLookupSolver = true;
			else if ( dynamic_cast< CHLSL_Solver_Flashlight* >( m_hSolverStack_PixelShader[i] ) )
				bHasLookupSolver = true;
		}
		Assert( bHasLookupSolver == bHasTextureIdentifier );
	}
#endif

	for ( int i = 0; i < m_hNodeList.Count(); i++ )
		m_hNodeList[ i ]->PostSolverUpdated();

#if DO_SOLVING
	OnSolvingFinsihed();
#endif

	m_bDirtySolvers = false;

	MakeHistoryDirty();

	hSolverCmds.PurgeAndDeleteElements();

	return true;
}

void CNodeView::InvokeCreateSolvers( CUtlVector< CBaseNode* > &m_hNodeBottomList,
										CUtlVector< CBaseNode* > &m_hNodeOutList,
										CUtlVector< CBaseContainerNode* > &m_hContainerStack,
										GenericShaderData *ShaderData )
{
	CUtlVector< CBaseNode* > m_hList_Shader_Top;
	CBaseContainerNode *activeContainer = NULL;
	if ( m_hContainerStack.Count() )
		activeContainer = m_hContainerStack.Tail();

	for (;;)
	{
		bool bEarlyOut = false; // avoid one sort of limbo
		for ( int i = 0; i < m_hNodeBottomList.Count(); i++ )
		{
			CBaseNode *n_SHADER_Bottom = m_hNodeBottomList[ i ];
			n_SHADER_Bottom->Recursive_AddTailNodes( m_hList_Shader_Top, true, true, activeContainer );
		}

		if ( activeContainer != NULL )
		{
			bool bHasPriorityNodes = false;
			CUtlVector< CBaseNode* >hDirectChildren;
			for ( int i = 0; i < m_hList_Shader_Top.Count(); i++ )
			{
				CBaseNode *pContainerTop = m_hList_Shader_Top[ i ];
				bool bIsSubChild = false;
				for ( int c = 0; c < activeContainer->GetNumChildren(); c++ )
				{
					CBaseContainerNode *pCCCHild = activeContainer->GetChild(c)->GetAsContainer();
					if ( !pCCCHild )
						continue;
					if ( pCCCHild->HasChild( pContainerTop ) )
						bIsSubChild = true;
				}
				if ( !bIsSubChild )
				{
					bHasPriorityNodes = true;
					hDirectChildren.AddToTail( pContainerTop );
				}
			}
			if ( bHasPriorityNodes )
			{
				m_hList_Shader_Top.Purge();
				m_hList_Shader_Top.AddVectorToTail( hDirectChildren );
				hDirectChildren.Purge();
			}
		}

		//Msg( "::::: input TOP nodes %i\n", m_hList_VertexShader_Top.Count() );
		bool bProcessedContainer = false;
		for ( int i = 0; i < m_hList_Shader_Top.Count(); i++ )
		{
			CBaseNode *n_SHADER_Top = m_hList_Shader_Top[ i ];
			//Assert( !n_SHADER_Top->GetAsContainer() );

			CUtlVector< CBaseContainerNode* > curContainers;
			
			n_SHADER_Top->ListContainersChronologically( curContainers );

			CBaseContainerNode *pCurContainer = n_SHADER_Top->GetAsContainer();
			//if ( pCurContainer != NULL &&
			//	pCurContainer->GetNumSolvers() )
			//	pCurContainer = NULL;

			if ( pCurContainer != NULL &&
				!curContainers.HasElement( pCurContainer) )
				curContainers.AddToTail( pCurContainer );

			if ( n_SHADER_Top->GetErrorLevel() == ERRORLEVEL_NONE ||
				pCurContainer != NULL )
			{
				for ( int c = 0; c < curContainers.Count(); c++ )
				{
					CBaseContainerNode *pC = curContainers[c];
					Assert( pC->GetAsContainer() );
					if ( m_hContainerStack.HasElement( pC ) )
						continue;
					if ( !pC->IsSolvable( true ) )
						continue;
					bool bIsCond = dynamic_cast< CNodeCondition* >( pC ) != NULL;
					if ( !bIsCond && pC->GetNumSolvers() > 0 )
						continue;

					m_hContainerStack.AddToTail( pC );
					pC->Solve_ContainerEntered();
					//pC->Solve_ContainerCreateVariables();
					m_hNodeOutList.AddToTail( pC );

					//int TargetHierachy = n_SHADER_Top->GetHierachyTypeIterateFullyRecursive();

					CUtlVector< CBaseNode* > hContainerBottom;
					Assert( pC->HasChild( n_SHADER_Top ) || pC == n_SHADER_Top );
					for ( int iChild = 0; iChild < pC->GetNumChildren(); iChild++ )
					{
						CBaseNode *pChild = pC->GetChild(iChild);
						m_hList_Shader_Top.FindAndRemove( pChild );

						//if ( pChild->GetHierachyTypeIterateFullyRecursive() != TargetHierachy )
						//	continue;
						pChild->Recursive_AddTailNodes( hContainerBottom, false, false, pC, true );
					}
					//i--;
					InvokeCreateSolvers( hContainerBottom, m_hNodeOutList, m_hContainerStack, ShaderData );
					hContainerBottom.Purge();

					m_hNodeOutList.AddToTail( pC );
					pC->Solve_ContainerLeft();
					m_hContainerStack.FindAndRemove( pC );

					bProcessedContainer = true;
				}

				//if ( pCurContainer )
				//	bProcessedContainer = true;
			}

			curContainers.Purge();

			if ( bProcessedContainer )
				continue;
			//Msg( "::::::: calling solve on: %i\n", n_VS_Top->GetNodeType() );
			if ( n_SHADER_Top->InvokeCreateSolvers(ShaderData) )
			{
#if SHOW_SEDIT_ERRORS
				Assert( !m_hNodeOutList.IsValidIndex( m_hNodeOutList.Find( n_SHADER_Top ) ) );
#endif
				m_hNodeOutList.AddToTail( n_SHADER_Top );
			}
			else
			{
				bEarlyOut = true;
				break;
			}
		}

		if ( ( !m_hList_Shader_Top.Count() || bEarlyOut ) && !bProcessedContainer )
			break;

		m_hList_Shader_Top.Purge();
	}
	//Msg( "- nodes solved created: %i\n", solvers );

	m_hList_Shader_Top.Purge();
}

int CNodeView::CreatePPSolversFromFile( const char *fileName, CUtlVector<CHLSL_SolverBase*> &hOutput, GenericPPEData &config )
{
	int iError = CPostProcessingCache::PPE_OKAY;
	//CNodeView *pCalcView = new CNodeView( NULL, ::pEditorRoot, "" );
	CNodeView *pCalcView = new CNodeView( NULL, NULL, "" );
	const bool bOpenSuccessful = pCalcView->LoadFromFile( fileName, true );

	if ( !bOpenSuccessful || pCalcView->GetNumNodes() < 1 )
		iError = CPostProcessingCache::PPE_INVALID_INPUT;
	else if ( pCalcView->GetFlowgraphType() == CNodeView::FLOWGRAPH_POSTPROC )
	{
		CBaseNode *pn_IN = pCalcView->GetNodeFromType( HLSLNODE_POSTPROCESSING_INPUT );
		CBaseNode *pn_OUT = pCalcView->GetNodeFromType( HLSLNODE_POSTPROCESSING_OUTPUT );

		//pn_IN->OnUpdateHierachy( NULL, NULL );
		pCalcView->DoFullHierachyUpdate();
		//for ( int i = 0; i < pCalcView->GetNumNodes(); i++ )
		//{
		//	if ( pCalcView->GetNode(i)->GetErrorLevel() != errorl
		//}

		if ( !pn_IN || !pn_OUT ||
			pn_IN->GetErrorLevel() != ERRORLEVEL_NONE ||
			pn_OUT->GetErrorLevel() != ERRORLEVEL_NONE )
			iError = CPostProcessingCache::PPE_GRAPH_NOT_VALID;
		else
		{
			pCalcView->Think_UpdateSolvers();
			CopySolvers( pCalcView->AccessSolverStack_POSTPROC(), hOutput );

			Assert( pCalcView->m_PPEData );

			config = *pCalcView->m_PPEData;
		}
	}
	else
		iError = CPostProcessingCache::PPE_INVALID_GRAPH_TYPE;

	delete pCalcView;
	return iError;
}

GenericPPEData *CNodeView::AccessPPEConfig()
{
	Assert( m_PPEData );
	return m_PPEData;
}

void CNodeView::DoEnvmapOverride()
{
	if ( !m_ShaderData )
		return;

	if ( !m_ShaderData->IsPreview() || !pEditorRoot )
		return;

	if ( !m_ShaderData->shader ||
		!m_ShaderData->shader->pPS_Identifiers )
		return;

	const char *destTex = pEditorRoot->GetEnvmapOverride();
	if ( !destTex )
		return;

	for ( int i = 0; i < m_ShaderData->shader->pPS_Identifiers->hList_Textures.Count(); i++ )
	{
		SimpleTexture *tex = m_ShaderData->shader->pPS_Identifiers->hList_Textures[i];
		if ( tex->iTextureMode != HLSLTEX_ENVMAP )
			continue;

		if ( tex->szTextureName != NULL )
			AllocCheck_FreeS( "szTextureName" );

		delete [] tex->szTextureName;
		tex->szTextureName = new char[ Q_strlen( destTex ) + 1 ];
		Q_strcpy( tex->szTextureName, destTex );
		AllocCheck_AllocS( "szTextureName" );
	}
}


void CNodeView::DestroyAllSolvers()
{
	DestroySolverStack(m_hSolverStack_VertexShader);
	DestroySolverStack(m_hSolverStack_PixelShader);
	DestroySolverStack(m_hSolverStack_NoHierachy);
	DestroySolverStack(m_hSolverStack_PostProcessing);
}
CUtlVector< CHLSL_SolverBase* > &CNodeView::AccessSolverStack_VS()
{
	return m_hSolverStack_VertexShader;
}
CUtlVector< CHLSL_SolverBase* > &CNodeView::AccessSolverStack_PS()
{
	return m_hSolverStack_PixelShader;
}
CUtlVector< CHLSL_SolverBase* > &CNodeView::AccessSolverStack_UNDEFINED()
{
	return m_hSolverStack_NoHierachy;
}
CUtlVector< CHLSL_SolverBase* > &CNodeView::AccessSolverStack_POSTPROC()
{
	return m_hSolverStack_PostProcessing;
}

bool CNodeView::IsMouseOver()
{
	//int sx,sy;
	//GetSize( sx, sy );
	//UpdateMousePos();
	//int px = Mx;
	//int py = My;
	//ScreenToLocal( px, py );
	//return px >= 0 && py >= 0 && px <= sx && py <= sy;

	return GetVPanel() == input()->GetMouseOver();
}

void CNodeView::ResetView()
{
	m_vecMenuCreationPosition.Init();

	ResetView_User();

	m_vecPosition.Init();
	m_flZoom = m_flZoomGoal;

	StopDrag( iArmDrag == DRAG_NODES );
	iArmDrag = DRAG_NONE;

	UpdateMousePos();
	MxOld = Mx;
	MyOld = My;
	MxDelta = MyDelta = 0;

	if ( pJackLast )
		pJackLast->SetFocus( false );
	pJackLast = NULL;
	bOverrideNodeIndices = false;
}
void CNodeView::ResetView_User( bool bInitial )
{
	if ( bInitial )
	{
		int x,y;
		engine->GetScreenSize(x,y);
		float relative = 592.0f / y;
		m_flZoomGoal = clamp( relative * 1.0f, VIEWZOOM_IN_MAX, VIEWZOOM_OUT_MAX );
		m_vecPositionGoal.Init();

		m_flZoom = VIEWZOOM_OUT_MAX;
		return;
	}

	int nsx, nsy;
	GetSize( nsx, nsy );
	Vector4D bounds;
	GetGraphBoundaries( bounds );

	Assert( bounds.z > bounds.x );
	Assert( bounds.w > bounds.y );

	m_vecPositionGoal.Init( -1.0f * bounds.x, bounds.y );
	m_vecPositionGoal.x -= (bounds.z - bounds.x) * 0.5f;
	m_vecPositionGoal.y += (bounds.w - bounds.y) * 0.5f;

	float delta_horizontal = (bounds.z - bounds.x) / nsx;
	float delta_vertical = (bounds.w - bounds.y) / nsy;
	m_flZoomGoal = max( delta_horizontal, delta_vertical ) + 0.2f;
	m_flZoomGoal = clamp( m_flZoomGoal, VIEWZOOM_IN_MAX, VIEWZOOM_OUT_MAX );
}
bool CNodeView::IsCursorOutOfBounds( Vector2D *delta )
{
	UpdateMousePos();

	int w,t;
	GetSize(w,t);

	if ( delta )
	{
		delta->Init();

		Vector2D mid( w * 0.5f, t * 0.5f );
		Vector2D cur( Mx, My );

		Vector2D _min( 0, t );
		Vector2D _max( w, 0 );

		ToNodeSpace( mid );
		ToNodeSpace( cur );
		ToNodeSpace( _min );
		ToNodeSpace( _max );

		Vector rstart( mid.x, mid.y, 0 );
		Vector rend( cur.x, cur.y, 0 );
		Ray_t ray;
		ray.Init( rstart, rend );

		Vector bmin( _min.x, _min.y, -10 );
		Vector bmax( _max.x, _max.y, 10 );

		float out = 0;
		CBaseTrace tr;
		if ( IntersectRayWithBox( ray, bmin, bmax, 1.0f, &tr, &out ) )
		{
			Vector intersect = rstart + (rend - rstart) * out;
			rend -= intersect;

			delta->Init( rend.x, rend.y );
		}
	}

	if ( Mx < 0 || My < 0 )
		return true;
	if ( Mx > w || My > t )
		return true;

	if ( delta )
		delta->Init();

	return false;
}

float CNodeView::GetZoomScalar()
{
	return 1.0f / max( 0.001f, m_flZoom );
}
void CNodeView::SetZoomScalar( float zoom )
{
	m_flZoom = zoom;
}
void CNodeView::ToNodeSpace( Vector2D &pos )
{
	Vector2D panelMid( GetWide() * 0.5f, GetTall() * 0.5f );
	Vector2D origin = panelMid + m_vecPosition;

	pos -= origin;

	Vector2D panelMidNodeSpace = panelMid - origin;
	Vector2D delta( pos - panelMidNodeSpace );

	delta *= m_flZoom;

	pos = panelMidNodeSpace + delta;
	pos.y *= -1.0f;
}
void CNodeView::ToPanelSpace( Vector2D &pos )
{
#if 1
	Vector2D panelMid( GetWide(), GetTall() );
	panelMid *= 0.5f * m_flZoom;
	Vector2D origin = panelMid + m_vecPosition;

	Vector2D MidPanelSpace = panelMid;
	//ToNodeSpace( MidPanelSpace );

	Vector2D delta( pos - MidPanelSpace );
	//delta *= 1.0f / max( 0.001f, m_flZoom );
	pos = MidPanelSpace + delta;

	pos.x += origin.x;
	pos.y -= origin.y;
	pos.y *= -1.0f;
	pos *= 1.0f / max( 0.001f, m_flZoom );
#else
	Vector2D panelMid( 0.5f * GetWide(), 0.5f * GetTall() );
	Vector2D origin = panelMid + m_vecPosition;

	Vector2D MidPanelSpace = panelMid;
	ToNodeSpace( MidPanelSpace );

	Vector2D delta( pos - MidPanelSpace );
	delta *= 1.0f / max( 0.001f, m_flZoom );
	pos = MidPanelSpace + delta;

	pos.x += origin.x;
	pos.y -= origin.y;
	pos.y *= -1.0f;
#endif
}
void CNodeView::GetGraphBoundaries( Vector4D &out )
{
	if ( !GetNumNodes() )
		out.Init( -100, -100, 100, 100 );
	else
	{
		CBaseNode *node = GetNode( 0 );
		Vector2D local_min = node->GetBoundsMinNodeSpace();
		Vector2D local_max = node->GetBoundsMaxNodeSpace();
		out.Init( local_min.x, local_min.y, local_max.x, local_max.y );
	}

	for ( int i = 1; i < GetNumNodes(); i++ )
	{
		CBaseNode *node = GetNode( i );
		
		Vector2D local_min = node->GetBoundsMinNodeSpace();
		Vector2D local_max = node->GetBoundsMaxNodeSpace();

		out.x = min( out.x, local_min.x );
		out.y = min( out.y, local_min.y );
		out.z = max( out.z, local_max.x );
		out.w = max( out.w, local_max.y );
	}
}
Vector2D CNodeView::GetMousePosInNodeSpace()
{
	UpdateMousePos();
	Vector2D mN( Mx, My );
	ToNodeSpace(mN);
	return mN;
}

void CNodeView::OnThink(void)
{
	UpdateMousePos();
	AccumulateMouseMove();

	BaseClass::OnThink();

	Think_HighlightJacks();
	Think_UpdateCursorIcon();
	Think_SmoothView();
	Think_Drag();

	Think_UpdateSolvers();
	Think_UpdateHistory();

	Think_CreateTooltip();
	SaveMousePos();

	UpdateUserErrorLayouts();
}
void CNodeView::Think_SmoothView()
{
	for ( int i = 0; i < 2; i++ )
	{
		float delta = ( m_vecPositionGoal[i] - m_vecPosition[i] );
		float move = delta * gpGlobals->frametime * VIEWSPEED_PAN;
		if ( abs(move) > abs(delta) )
			move = delta;
		m_vecPosition[i] += move;
	}
	float delta = ( m_flZoomGoal - m_flZoom );
	float move = delta * gpGlobals->frametime * VIEWSPEED_ZOOM;
	if ( abs(move) > abs(delta) )
		move = delta;
	m_flZoom += move;
}
void CNodeView::Think_Drag()
{
	if ( !IsInDrag() )
		return;

	bool bNodeDrag = iArmDrag == DRAG_NODES;
	bool bSBoxDrag = iArmDrag == DRAG_SBOX;
	bool bBridgeDrag = iArmDrag == DRAG_BRDIGE;

	if ( bBridgeDrag && m_pCurBridge )
	{
		Vector2D target = GetMousePosInNodeSpace();
		CJack *curJack = GetBridgeSnapJack();
		if ( curJack )
		{
			target = curJack->GetCenter();
			if ( !curJack->IsInput() )
				target.x = curJack->GetBoundsMax().x;
			else
				target.x = curJack->GetBoundsMin().x;
		}

		m_pCurBridge->SetTemporaryTarget( target );
	}

	if ( bSBoxDrag || bBridgeDrag )
	{
		Think_PullView();
		return;
	}

	Vector2D curpos( Mx, My );
	Vector2D oldpos( MxOld, MyOld );

	ToNodeSpace(curpos);
	ToNodeSpace(oldpos);

	Vector2D delta = curpos - oldpos;

	if ( bNodeDrag )
	{
		Vector2D d = Think_PullView();
		DragSelection( delta + d );
		return;
	}

	delta.y *= -1.0f;

	m_vecPositionGoal += delta;
}
Vector2D CNodeView::Think_PullView()
{
	Vector2D delta;
	if ( !IsCursorOutOfBounds( &delta ) )
		return Vector2D(0,0);

	float len = delta.NormalizeInPlace();
	len = min( editor_pull_speed_max.GetFloat() / GetZoomScalar(), len );
	delta *= len;

	delta *= gpGlobals->frametime * editor_pull_speed_mul.GetFloat();

	m_vecPositionGoal -= Vector2D( delta.x, -delta.y );
	return delta;
}
void CNodeView::Think_HighlightJacks()
{
	CJack *cur = NULL;
	if ( !IsInDrag() || iArmDrag != DRAG_BRDIGE )
	{
		float t = BRIDGE_DRAG_TOLERANCE;
		cur = GetJackUnderCursor(&t);
	}
	else
		cur = GetBridgeSnapJack();

	if ( IsInDrag() && iArmDrag != DRAG_BRDIGE )
		cur = NULL;

	for ( int i = 0; cur != NULL && i < cur->GetNumBridges(); i++ )
		cur->GetBridge(i)->SetTemporaryColor( CBridge::TMPCOL_TRANS );

	if ( cur == pJackLast )
		return;

	if ( pJackLast )
		pJackLast->SetFocus( false );

	if ( cur )
		cur->SetFocus( true );

	pJackLast = cur;
}
void CNodeView::Think_UpdateCursorIcon()
{
	int hoverCursor = 0;
	int dcTarget = dc_user;

	Vector2D mPos( Mx, My );
	ToNodeSpace( mPos );

	if ( !pJackLast && IsMouseOver() && !pEditorRoot->IsPainting() )
	{
		if ( IsInDrag() && iArmDrag == DRAG_NODES )
		{
			dcTarget = m_iCursorLast;
		}
		else
		{
			for ( int i = 0; i < m_hContainerNodes.Count(); i++ )
			{
				hoverCursor = m_hContainerNodes[i]->IsInBorderBounds( mPos );
				if ( hoverCursor )
					break;
			}
		}
	}

	switch ( hoverCursor )
	{
	default:
		break;
	case CBORDER_TOP_LEFT:
	case CBORDER_BOTTOM_RIGHT:
		dcTarget = dc_sizenwse;
		break;
	case CBORDER_TOP_RIGHT:
	case CBORDER_BOTTOM_LEFT:
		dcTarget = dc_sizenesw;
		break;
	case CBORDER_TOP:
	case CBORDER_BOTTOM:
		dcTarget = dc_sizens;
		break;
	case CBORDER_RIGHT:
	case CBORDER_LEFT:
		dcTarget = dc_sizewe;
		break;
	}
	if ( m_iCursorLast == dcTarget )
		return;

	m_iCursorLast = dcTarget;
	input()->SetCursorOveride( dcTarget );
}

void CNodeView::Think_CreateTooltip()
{
	if ( m_pTooltip )
		return;

	if ( m_ContextMenu && m_ContextMenu->IsVisible() )
		return;

	if ( m_lTime_MouseLastMoved <= 0 )
		return;

	long curTime = system()->GetTimeMillis();
	bool bShouldCreateTooltip = curTime - m_lTime_MouseLastMoved > 1000 && pEditorRoot->ShouldShowTooltips();

	if ( bShouldCreateTooltip )
	{
		CBaseNode *pNode = GetNodeUnderCursor();
		if ( pNode )
			CreateTooltip( pNode->GetNodeType() );
	}
}

void CNodeView::UpdateMousePos()
{
	input()->GetCursorPosition( Mx, My );
	ScreenToLocal(Mx,My);
}
void CNodeView::AccumulateMouseMove()
{
	MxDelta = Mx - MxOld;
	MyDelta = My - MyOld;
}
void CNodeView::SaveMousePos()
{
	MxOld = Mx;
	MyOld = My;
}

void CNodeView::ForceFocus()
{
	input()->SetMouseFocus( GetVPanel() );
	if ( m_ContextMenu.Get() )
		m_ContextMenu->OnKillFocus();
}
void CNodeView::OnKeyCodeTyped(KeyCode code)
{
	DestroyTooltip();

	OnParentKeyCodeTyped( code );
}
void CNodeView::OnKeyCodePressed ( vgui::KeyCode code )
{
	OnParentKeyCodePressed( code );
}
void CNodeView::OnKeyCodeReleased( vgui::KeyCode code )
{
	OnParentKeyCodeReleased( code );
}
void CNodeView::OnParentKeyCodeTyped(KeyCode code)
{
	if ( !AllowKeyInput() ) return;
}
void CNodeView::OnParentKeyCodePressed ( vgui::KeyCode code )
{
	if ( !AllowKeyInput() ) return;

	if ( input()->IsKeyDown( KEY_LCONTROL ) || input()->IsKeyDown( KEY_RCONTROL ) )
	{
		if ( code == KEY_C )
			CopySelection();
		else if ( code == KEY_V )
		{
			Vector2D pos( Mx, My );
			ToNodeSpace( pos );
			PasteNodes( pEditorRoot->GetCopyNodes(), true, &pos );
		}
		else if ( code == KEY_X )
		{
			CopySelection();
			DeleteSelection();
		}
		else if ( code == KEY_Z )
		{
			const bool bShiftDown = input()->IsKeyDown( KEY_LSHIFT ) || input()->IsKeyDown( KEY_RSHIFT );
			HistoryAction( bShiftDown ? HACT_REDO : HACT_UNDO );
		}
		return;
	}

	switch ( code )
	{
	case KEY_DELETE:
		DeleteSelection();
		return;
	}
}
void CNodeView::OnParentKeyCodeReleased( vgui::KeyCode code )
{
	if ( !AllowKeyInput() ) return;
}
void CNodeView::OnMousePressed ( vgui::MouseCode code )
{
	DestroyTooltip();
	ForceFocus();

	BaseClass::OnMousePressed( code );

	if ( code == MOUSE_RIGHT )
	{
		iArmDrag = DRAG_VIEW;
	}
	else if ( code == MOUSE_LEFT )
	{
		CBaseNode *cur = GetNodeUnderCursor();
		float t = BRIDGE_DRAG_TOLERANCE;
		CJack *curJack = GetJackUnderCursor(&t);

		HandleSelectionInputPressed( cur );

		if ( curJack && ( !cur || cur == curJack->GetParentNode() ) )
		{
			CBaseNode *n = curJack->GetParentNode();
			if ( n )
			{
				bool bReArrange = curJack->IsInput() && curJack->GetNumBridges();
				iArmDrag = DRAG_BRDIGE;

				if ( !bReArrange )
					CreateDragBridge( curJack );
				else
					UpdateDragBridge( curJack, n );
			}
		}
		else if ( cur )
		{
			if ( cur->IsSelected() )
			{
				iArmDrag = DRAG_NODES;
				Vector2D mPos( Mx, My );
				ToNodeSpace( mPos );
				cur->OnLeftClick( mPos );
			}
		}
		else
		{
			m_vecSelectionBoxStart = GetMousePosInNodeSpace();
			iArmDrag = DRAG_SBOX;
		}
	}
}

void CNodeView::OnMouseDoublePressed(MouseCode code)
{
	ForceFocus();

	BaseClass::OnMouseDoublePressed( code );

	if ( code == MOUSE_LEFT )
	{
		CBaseNode *cur = GetNodeUnderCursor();
		if ( cur )
		{
			CreatePropertyDialog( cur );
		}
	}
}
void CNodeView::OnMouseReleased( vgui::MouseCode code )
{
	BaseClass::OnMouseReleased( code );

	const bool bNodeDrag = iArmDrag == DRAG_NODES;
	const bool bSBoxDrag = iArmDrag == DRAG_SBOX;
	const bool bBridgeDrag = iArmDrag == DRAG_BRDIGE;

	iArmDrag = DRAG_NONE;

	if ( IsInDrag() )
	{
		if ( bSBoxDrag )
		{
			HandleSelectionInputReleasedBox();
		}
		else if ( bBridgeDrag && m_pCurBridge )
		{
			CJack *tJ = GetBridgeSnapJack();

			if ( tJ && tJ->GetParentNode() )
			{
				if ( tJ->IsInput() && tJ->GetNumBridges() )
				{
					Assert( tJ->GetNumBridges() == 1 );
					tJ->DisconnectBridges();
				}
				FinishDragBridge( tJ );
			}
			else
				KillDragBridge();
		}

		StopDrag( bNodeDrag );
		return;
	}
	else if ( m_pCurBridge )
	{
		KillDragBridge();
	}

	if ( code == MOUSE_RIGHT )
	{
		CBaseNode *cur = GetNodeUnderCursor();
		float t = BRIDGE_DRAG_TOLERANCE;
		CJack *curJack = GetJackUnderCursor(&t);

		if ( curJack && ( !cur || cur == curJack->GetParentNode() ) )
		{
			curJack->PurgeBridges();
			return;
		}
		else if ( cur )
		{
			if ( !cur->IsSelected() )
			{
				DeselectAllNodes( cur );
				cur->SetSelected( true );
			}
		}
		else
			DeselectAllNodes();

		CreateContextMenu( GetFirstSelectedNode() );
	}
	if ( code == MOUSE_LEFT )
	{
		CBaseNode *cur = GetNodeUnderCursor();

		HandleSelectionInputReleased( cur );
	}
}
void CNodeView::OnCursorMoved( int x, int y )
{
	BaseClass::OnCursorMoved(x,y);

	DestroyTooltip();

	m_lTime_MouseLastMoved = system()->GetTimeMillis();

	if ( iArmDrag && ( x || y ) && !IsInDrag() )
	{
		bool bNodeDrag = iArmDrag == DRAG_NODES;
		BeginDrag( bNodeDrag );
	}
}
void CNodeView::OnMouseWheeled( int delta )
{
	if ( !IsMouseOver() )
		return;

	DestroyTooltip();

	BaseClass::OnMouseWheeled(delta);

	if ( IsInDrag() && iArmDrag == DRAG_NODES )
		return;

	Vector2D oldPos = GetMousePosInNodeSpace();

	float flZoomDelta = delta * VIEWZOOM_SPEED * Bias( RemapValClamped(m_flZoom,0,VIEWZOOM_OUT_MAX,0,1), 0.6f );
	float flZoomOld = m_flZoom;

	m_flZoom -= flZoomDelta;
	m_flZoomGoal -= flZoomDelta;
	m_flZoom = clamp( m_flZoom, VIEWZOOM_IN_MAX, VIEWZOOM_OUT_MAX );
	m_flZoomGoal = clamp( m_flZoomGoal, VIEWZOOM_IN_MAX, VIEWZOOM_OUT_MAX );

	Vector2D newPos = GetMousePosInNodeSpace();

	m_flZoom = flZoomOld;

	Vector2D deltaPos = newPos - oldPos;

	if ( delta < 0 )
		deltaPos *= -1.0f;

	deltaPos.y *= -1.0f;

	m_vecPositionGoal += deltaPos;
}

void CNodeView::OnCursorEntered()
{
	m_lTime_MouseLastMoved = 0;
	DestroyTooltip();
}
void CNodeView::OnCursorExited()
{
	m_lTime_MouseLastMoved = 0;
	DestroyTooltip();
}

void CNodeView::CreateTooltip( int iNodeIndex )
{
	DestroyTooltip();

	KeyValues *pNodeHelp = pEditorRoot->GetNodeHelpContainer();
	if ( !pNodeHelp )
		return;

	KeyValues *pInfo = pNodeHelp->FindKey( VarArgs( "%i", iNodeIndex ) );
	if ( !pInfo )
		return;

	char *pszName = CKVPacker::ConvertKVSafeString( pInfo->GetString( "name" ), false );
	char *pszInfo = CKVPacker::ConvertKVSafeString( pInfo->GetString( "info" ), false );
	char *pszCode = CKVPacker::ConvertKVSafeString( pInfo->GetString( "code" ), false );

	if ( !pszName && !pszInfo && !pszCode )
		return;

	CSmartObject *pO = new CSmartObject( ACOMP_VAR, pszName, NULL, NULL );
	AutoCopyStringPtr( pszInfo, &pO->m_pszHelptext );
	if ( pszCode && Q_strlen(pszCode) )
	AutoCopyStringPtr( VarArgs("Example: %s",pszCode), &pO->m_pszSourceFile );

	delete [] pszName;
	delete [] pszInfo;
	delete [] pszCode;

	m_pTooltip = new CSmartTooltip( this, "nodetooltip" );
	m_pTooltip->Init( pO );
	m_pTooltip->InvalidateLayout( true );

	int mx, my, sx, sy, tsx, tsy;
	input()->GetCursorPosition( mx, my );
	mx += 16;

	surface()->GetScreenSize( sx, sy );
	m_pTooltip->GetSize( tsx, tsy );

	mx -= max( 0, tsx + mx - sx );
	my -= max( 0, tsy + my - sy );

	//ScreenToLocal( mx, my );
	m_pTooltip->SetPos( mx, my );

	delete pO;
}

void CNodeView::DestroyTooltip()
{
	if ( !m_pTooltip )
		return;

	m_pTooltip->MarkForDeletion();
	m_pTooltip = NULL;

	m_lTime_MouseLastMoved = 0;
}

void CNodeView::BeginDrag( bool bNodes )
{
	bInDrag = true;
	input()->SetMouseCapture( GetVPanel() );

	Assert( !m_hNodesInMove.Count() );
	if ( bNodes )
	{
		m_hNodesInMove.Purge();
		Vector2D mouseInNodeSpace( MxOld, MyOld );
		ToNodeSpace( mouseInNodeSpace );
		CBaseNode *cur = GetNodeUnderPos( mouseInNodeSpace );

		for ( int i = 0; i < m_hNodeList.Count(); i++ )
			if ( m_hNodeList[i]->IsSelected() )
				m_hNodesInMove.AddToTail( m_hNodeList[i] );
		for ( int i = 0; i < m_hNodesInMove.Count(); i++ )
			if ( m_hNodesInMove[i]->MustDragAlone() )
			{
				if ( cur == m_hNodesInMove[i] )
				{
					CBaseNode *SingleNode = m_hNodesInMove[i];
					DeselectAllNodes( SingleNode );
					m_hNodesInMove.Purge();
					m_hNodesInMove.AddToTail( SingleNode );
					break;
				}
				else
				{
					m_hNodesInMove[ i ]->SetSelected( false );
					m_hNodesInMove.Remove( i );
					i--;
				}
			}
		for ( int i = 0; i < m_hNodesInMove.Count(); i++ )
			m_hNodesInMove[i]->OnDragStart();
	}
}
bool CNodeView::IsInDrag()
{
	return bInDrag;
}
void CNodeView::StopDrag( bool bNodes )
{
	bInDrag = false;
	input()->SetMouseCapture( NULL );

	if ( bNodes )
	{
		for ( int i = 0; i < m_hNodesInMove.Count(); i++ )
			m_hNodesInMove[i]->OnDragEnd();
		m_hNodesInMove.Purge();

		MakeHistoryDirty();
	}
}
void CNodeView::HandleSelectionInputPressed( CBaseNode *pNode )
{
	bool bToggle = input()->IsKeyDown( KEY_LCONTROL ) || input()->IsKeyDown( KEY_RCONTROL );
	bool bAdd = input()->IsKeyDown( KEY_LSHIFT ) || input()->IsKeyDown( KEY_RSHIFT );
	if ( bAdd )
		bToggle = false;
	bool bAccum = bAdd || bToggle;

	if ( pNode )
	{
		CBaseContainerNode *pContainer = pNode->GetAsContainer();

		bool bWasSelected = pNode->IsSelected();
		if ( !bWasSelected && !bAccum )
			DeselectAllNodes( pNode );

		if ( pContainer != NULL )
		{
			pContainer->UpdateOnMove();

			if ( pContainer->ShouldSelectChildrenOnClick() )
				pContainer->SelectAllInBounds();
		}

		if ( bToggle )
			pNode->ToggleSelection();
		else
			pNode->SetSelected( true );

		MoveNodeToFront( pNode );
	}
}
void CNodeView::HandleSelectionInputReleased( CBaseNode *pNode )
{
	bool bToggle = input()->IsKeyDown( KEY_LCONTROL ) || input()->IsKeyDown( KEY_RCONTROL );
	bool bAdd = input()->IsKeyDown( KEY_LSHIFT ) || input()->IsKeyDown( KEY_RSHIFT );
	if ( bAdd )
		bToggle = false;
	bool bAccum = bAdd || bToggle;

	if ( pNode )
	{
		CBaseContainerNode *pContainer = pNode->GetAsContainer();

		if ( !bAccum )
			DeselectAllNodes( pNode );

		if ( pContainer != NULL )
		{
			pContainer->UpdateOnMove();

			if ( pContainer->ShouldSelectChildrenOnClick() )
				pContainer->SelectAllInBounds();
		}
	}
	else if ( !bAccum )
		DeselectAllNodes();
}
void CNodeView::HandleSelectionInputReleasedBox()
{
	bool bToggle = input()->IsKeyDown( KEY_LCONTROL ) || input()->IsKeyDown( KEY_RCONTROL );
	bool bAdd = input()->IsKeyDown( KEY_LSHIFT ) || input()->IsKeyDown( KEY_RSHIFT );
	if ( bAdd )
		bToggle = false;
	bool bAccum = bAdd || bToggle;

	Vector2D sstart = m_vecSelectionBoxStart;
	Vector2D ssend = GetMousePosInNodeSpace();
	Vector smin( min( sstart.x, ssend.x ),
				min( sstart.y, ssend.y ), -10 );
	Vector smax( max( sstart.x, ssend.x ),
				max( sstart.y, ssend.y ), 10 );

	CUtlVector< CBaseNode* >m_hSelection;
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *n = m_hNodeList[i];

		Vector2D nmin = n->GetSelectionBoundsMinNodeSpace(); //GetBoundsMinNodeSpace();
		Vector2D nmax = n->GetSelectionBoundsMaxNodeSpace(); //GetBoundsMaxNodeSpace();
		Vector n1( nmin.x, nmin.y, -10 );
		Vector n2( nmax.x, nmax.y, 10 );
		
		if ( IsBoxIntersectingBox( smin, smax, n1, n2 ) )
		{
			m_hSelection.AddToTail( n );
		}
	}

	if ( !bAccum )
		DeselectAllNodes();
	for ( int i = 0; i < m_hSelection.Count(); i++ )
	{
		CBaseNode *n = m_hSelection[i];
		if ( bToggle )
			n->ToggleSelection();
		else
			n->SetSelected( true );
	}

	m_hSelection.Purge();
}

CBaseNode *CNodeView::GetNodeUnderCursor()
{
	UpdateMousePos();
	Vector2D mouseInNodeSpace( Mx, My );
	ToNodeSpace( mouseInNodeSpace );
	return GetNodeUnderPos( mouseInNodeSpace );
}
CBaseNode *CNodeView::GetNodeUnderPos(Vector2D pos)
{
	CBaseNode *p = NULL;

	for ( int i = m_hNodeList.Count() - 1; i >= 0 ; i-- )
	{
		CBaseNode *n = m_hNodeList[i];
		if ( n->IsWithinBounds_Base( pos ) )
		{
			p = n;
			break;
		}
	}

	return p;
}
CJack *CNodeView::GetJackUnderCursor( float *tolerance, bool bInputs, bool bOutputs )
{
	UpdateMousePos();
	Vector2D mouseInNodeSpace( Mx, My );
	ToNodeSpace( mouseInNodeSpace );
	return GetJackUnderPos( mouseInNodeSpace, tolerance, bInputs, bOutputs );
}
CJack *CNodeView::GetJackUnderPos(Vector2D pos, float *tolerance, bool bInputs, bool bOutputs )
{
	if ( !IsMouseOver() )
		return NULL;

	CJack *j = NULL;
	for ( int i = m_hNodeList.Count() - 1; i >= 0 ; i-- )
	{
		CBaseNode *n = m_hNodeList[i];
		float bestdist = 99999;
		CJack *bestJack = NULL;

		if ( bOutputs )
		{
			for ( int x = 0; x < n->GetNumJacks_Out(); x++ )
			{
				CJack *tmp = n->GetJack_Out(x);
				if ( tmp->IsWithinBounds_Base( pos ) )
				{
					j = tmp;
					break;
				}
				if (tolerance)
				{
					Vector2D c = tmp->GetCenter();
					c -= pos;
					float dist = c.Length();
					if ( dist < *tolerance && dist < bestdist )
					{
						bestdist = dist;
						bestJack = tmp;
					}
				}
			}
		}
		if ( bInputs )
		{
			for ( int x = 0; x < n->GetNumJacks_In(); x++ )
			{
				CJack *tmp = n->GetJack_In(x);
				if ( tmp->IsWithinBounds_Base( pos ) )
				{
					j = tmp;
					break;
				}
				if (tolerance)
				{
					Vector2D c = tmp->GetCenter();
					c -= pos;
					float dist = c.Length();
					if ( dist < *tolerance && dist < bestdist )
					{
						bestdist = dist;
						bestJack = tmp;
					}
				}
			}
		}

		if ( bestJack && !j )
			j = bestJack;

		if ( j )
			break;
	}
	return j;
}
void CNodeView::DeselectAllNodes( CBaseNode *pIgnore )
{
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *n = m_hNodeList[i];
		if ( n == pIgnore )
			continue;
		n->SetSelected( false );
	}
}
void CNodeView::SelectNodes( CUtlVector< CBaseNode* > &m_hNodes, bool bAdd )
{
	if ( !bAdd )
		DeselectAllNodes();
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *n = m_hNodeList[i];
		if ( m_hNodes.Find( n ) != m_hNodes.InvalidIndex() )
			n->SetSelected( true );
	}
}
bool CNodeView::IsSelectionBiggerThanOne()
{
	int num = 0;
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *n = m_hNodeList[i];
		if ( n->IsSelected() )
			num++;
	}
	return num > 1;
}
CBaseNode *CNodeView::GetFirstSelectedNode()
{
	for ( int i = m_hNodeList.Count()-1; i >= 0; i-- )
	{
		CBaseNode *n = m_hNodeList[i];
		if ( n->IsSelected() )
			return n;
	}
	return NULL;
}
void CNodeView::DragSelection( Vector2D delta )
{
	//CUtlVector< CBaseNode* >hSelection;
	//for ( int i = 0; i < m_hNodeList.Count(); i++ )
	//	if ( m_hNodeList[i] )
	//		hSelection.AddToTail( m_hNodeList[i] );

	//for ( int i = 0; i < hSelection.Count(); i++ )
	//	if ( hSelection[i]->MustDragAlone() )
	//	{
	//		CBaseNode *SingleNode = hSelection[i];
	//		DeselectAllNodes( SingleNode );
	//		hSelection.Purge();
	//		hSelection.AddToTail( SingleNode );
	//		break;
	//	}
	
	//for ( int i = 0; i < hSelection.Count(); i++ )
	for ( int i = 0; i < m_hNodesInMove.Count(); i++ )
	{
		//CBaseNode *n = m_hNodeList[i];
		CBaseNode *n = m_hNodesInMove[i];
		//if ( !n->IsSelected() )
		//	continue;

		n->OnDrag( delta );

		//Vector2D pos = n->GetPosition();
		//pos += delta;
		//n->SetPosition( pos );
	}
	//hSelection.Purge();
}
void CNodeView::MoveNodeToFront( CBaseNode *p )
{
	if ( p == m_hNodeList.Tail() )
		return;

	int idx = m_hNodeList.Find( p );
	m_hNodeList.Remove( idx );
	m_hNodeList.AddToTail( p );
}
void CNodeView::CopySelection()
{
	if ( !AllowKeyInput() ) return;

	CUtlVector< CBaseNode* > m_hCopy;
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		if ( !m_hNodeList[i]->IsSelected() )
			continue;
		if ( m_hNodeList[i]->IsNodeCrucial() )
			continue;
		m_hCopy.AddToTail( m_hNodeList[i] );
	}

	pEditorRoot->SendCopyNodes( CopyNodes( m_hCopy ) );
	m_hCopy.Purge();
}
void CNodeView::PasteSelection()
{
	if ( !AllowKeyInput() ) return;
	//KeyValues *pKV = pEditorRoot->GetCopyNodes();
	//if ( !pKV )
	//	return;

	PasteNodes( pEditorRoot->GetCopyNodes(), true, &m_vecMenuCreationPosition );

	//CUtlVector< CBaseNode* > m_hPasted;
	//PasteNodes( m_vecMenuCreationPosition, pKV, &m_hPasted );
	//SelectNodes( m_hPasted, false );
}
void CNodeView::DeleteSelection()
{
	if ( !AllowKeyInput() ) return;

	CUtlVector< CBaseNode* > m_hDeleteThese;
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *n = m_hNodeList[i];
		if ( !n->IsSelected() )
			continue;
		if ( n->IsNodeCrucial() )
			continue;

		m_hNodesInMove.FindAndRemove( m_hNodeList[i] );
		m_hNodeList.Remove( i );
		n->MarkForDeletion();
		m_hDeleteThese.AddToTail( n );
		i--;
	}
	SaveDeleteVector( m_hDeleteThese );
}
void CNodeView::DisconnectSelection()
{
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *n = m_hNodeList[i];
		if ( !n->IsSelected() )
			continue;

		n->PurgeBridges();
	}
}

bool CNodeView::AllowKeyInput()
{
	if ( input()->GetAppModalSurface() )
		return false;
	if ( m_Properties.Get() )
		return false;

	return true;
}

void CNodeView::StartOverrideIndices( CUtlVector< CBaseNode* > &m_hNodes )
{
	bOverrideNodeIndices = true;
	m_hTmpNodeIndex.Purge();
	m_hTmpNodeIndex.AddVectorToTail( m_hNodes );
}
void CNodeView::FinishOverrideIndices()
{
	bOverrideNodeIndices = false;
	m_hTmpNodeIndex.Purge();
}
KeyValues *CNodeView::CopyNodes( CUtlVector< CBaseNode* > &m_hNodesToCopy, bool bCenterNodes )
{
	if ( m_hNodesToCopy.Count() < 1 )
		return NULL;
	Vector2D groupMin = m_hNodesToCopy[0]->GetBoundsMinNodeSpace();
	Vector2D groupMax = m_hNodesToCopy[0]->GetBoundsMaxNodeSpace();
	Vector2D groupMid;

	for ( int i = 1; i < m_hNodesToCopy.Count(); i++ )
	{
		Vector2D curmin = m_hNodesToCopy[i]->GetBoundsMinNodeSpace();
		Vector2D curmax = m_hNodesToCopy[i]->GetBoundsMaxNodeSpace();
		if ( curmin.x < groupMin.x )
			groupMin.x = curmin.x;
		if ( curmin.y < groupMin.y )
			groupMin.y = curmin.y;

		if ( curmax.x > groupMax.x )
			groupMax.x = curmax.x;
		if ( curmax.y > groupMax.y )
			groupMax.y = curmax.y;
	}

	groupMid = groupMin + ( groupMax - groupMin ) * 0.5f;

	StartOverrideIndices( m_hNodesToCopy );

	KeyValues *pKV = new KeyValues("Nodes");
	for ( int i = 0; i < m_hNodesToCopy.Count(); i++ )
	{
		pKV->AddSubKey( m_hNodesToCopy[i]->AllocateKeyValues( i ) );
	}
	if ( bCenterNodes )
		MoveNodes( -groupMid, pKV );

	FinishOverrideIndices();

	return pKV;
}
CBaseNode *CNodeView::AllocateSingleNode( KeyValues *pKV )
{
	int type = pKV->GetInt( "iType" );
	CBaseNode *pNode = SpawnNode( type );
	if ( !pNode )
		return NULL;

	Assert( pNode );
	pNode->RestoreFromKeyValues( pKV );
	return pNode;
}
void CNodeView::ConnectBridges( CUtlVector< CBaseNode* > &m_hInstantiatedNodes, KeyValues *pKV )
{
	StartOverrideIndices( m_hInstantiatedNodes );
	for ( int i = 0; i < m_hInstantiatedNodes.Count(); i++ )
	{
		CBaseNode *n = m_hInstantiatedNodes[i];
		char tmp[MAX_PATH];
		Q_snprintf( tmp, MAX_PATH, "Node_%03i", i );
		KeyValues *data = pKV->FindKey( tmp );
		if ( !data )
		{
			Warning( "no bridge info on instantiated node!\n" );
			continue;
		}
		n->RestoreFromKeyValues_CreateBridges( data );
	}
	FinishOverrideIndices();
}
void CNodeView::PasteNodes( KeyValues *pKV, bool bAutoSelect, Vector2D *origin, CUtlVector< CBaseNode* > *m_hNewNodes )
{
	if ( !pKV )
		return;

	int idx = 0;
	CUtlVector< CBaseNode* >m_hInstantiatedNodes;

	for (;;)
	{
		char tmp[MAX_PATH];
		Q_snprintf( tmp, MAX_PATH, "Node_%03i", idx );
		KeyValues *pKVNode = pKV->FindKey( tmp );
		idx++;
		if ( !pKVNode )
			break;

		CBaseNode *pNode = AllocateSingleNode( pKVNode );
		if ( !pNode )
			continue;

		if ( origin )
			MoveNodes( *origin, pNode );

		m_hInstantiatedNodes.AddToTail( pNode );
	}

	ConnectBridges( m_hInstantiatedNodes, pKV );

	if ( m_hNewNodes )
		m_hNewNodes->AddVectorToTail( m_hInstantiatedNodes );

	if ( bAutoSelect )
		SelectNodes( m_hInstantiatedNodes, false );

	m_hInstantiatedNodes.Purge();
}
KeyValues *CNodeView::BuildFullCanvas()
{
	KeyValues *pNodes = CopyNodes( m_hNodeList, false );
	if ( !pNodes )
		return NULL;

	FlowGraphType_t type = GetFlowgraphType();

	KeyValues *pCanvas = new KeyValues("canvas");
	// data version
	pCanvas->SetString( GetCanvasVersion_KeyName(), GetCanvasVersion_Current() );

	// graphtype
	pCanvas->SetInt( "flowgraph_type", type );

	// main config
	if ( type == FLOWGRAPH_HLSL )
	{
		KeyValues *pConfig = new KeyValues("config");
		pConfig->SetInt( "shadermodel", m_ShaderData->shader->iShaderModel );
		pConfig->SetInt( "cull_mode", m_ShaderData->shader->iCullmode );
		pConfig->SetInt( "alpha_blending", m_ShaderData->shader->iAlphablendmode );
		pConfig->SetInt( "depth_test", m_ShaderData->shader->iDepthtestmode );
		pConfig->SetInt( "depth_write", m_ShaderData->shader->iDepthwritemode );
		pConfig->SetInt( "srgb_write", m_ShaderData->shader->bsRGBWrite ? 1 : 0 );
		pCanvas->AddSubKey( pConfig );
	}
	else if ( type == FLOWGRAPH_POSTPROC )
	{
		KeyValues *pConfig = new KeyValues("config_ppe");
		pConfig->SetInt( "updatefbcopy", m_PPEData->bDoAutoUpdateFBCopy );
		pCanvas->AddSubKey( pConfig );
	}

	// nodes
	pCanvas->AddSubKey( pNodes );

	return pCanvas;
}
void CNodeView::RestoreFullCanvas( KeyValues *pCanvas )
{
	PurgeCanvas();

	FlowGraphType_t type = (FlowGraphType_t)pCanvas->GetInt( "flowgraph_type", FLOWGRAPH_HLSL );

	SetFlowgraphType( type );

	KeyValues *pNodes = pCanvas->FindKey( "nodes" );
	if ( pNodes )
		PasteNodes( pNodes, false );

	if ( type == FLOWGRAPH_HLSL )
	{
		KeyValues *pConfig = pCanvas->FindKey( "config" );
		if ( pConfig )
		{
			Assert( m_ShaderData );
			m_ShaderData->shader->iShaderModel = pConfig->GetInt( "shadermodel" );
			m_ShaderData->shader->iCullmode = pConfig->GetInt( "cull_mode" );
			m_ShaderData->shader->iAlphablendmode = pConfig->GetInt( "alpha_blending" );
			m_ShaderData->shader->iDepthtestmode = pConfig->GetInt( "depth_test" );
			m_ShaderData->shader->iDepthwritemode = pConfig->GetInt( "depth_write" );
			m_ShaderData->shader->bsRGBWrite = !!pConfig->GetInt( "srgb_write" );
		}
	}
	else if ( type == FLOWGRAPH_POSTPROC )
	{
		KeyValues *pConfig = pCanvas->FindKey( "config_ppe" );
		if ( pConfig )
		{
			m_PPEData->bDoAutoUpdateFBCopy = !!pConfig->GetInt( "updatefbcopy" );
		}
	}

	DoFullHierachyUpdate();

	if ( GetParent() != NULL )
		pEditorRoot->UpdateTabColor( this );
}
void CNodeView::SaveToFile( const char *fullpath )
{
	if ( fullpath == NULL )
		fullpath = GetShadername();

	if ( fullpath == NULL )
		return;

	KeyValues *pCanvas = BuildFullCanvas();
	if ( !pCanvas )
		return;

	pCanvas->SaveToFile( g_pFullFileSystem, fullpath, "MOD" );

	pCanvas->deleteThis();

	m_HistoryIndex_LastSaved = m_HistoryIndex;
}
bool CNodeView::LoadFromFile( const char *fullpath, bool bSuppressError )
{
	bool bSuccess = false;
	KeyValues *pCanvas = new KeyValues("canvas");

	if ( pCanvas->LoadFromFile( g_pFullFileSystem, fullpath, "MOD" ) )
	{
		const char *pCVName = pCanvas->GetString( GetCanvasVersion_KeyName() ); //, GetCanvasVersion_Current() );
		bool bAllowLoad = true;

		if ( pCVName && Q_strlen( pCVName ) > 0 )
		{
			bAllowLoad = false;
			for ( int i = 0; i < GetCanvasVersion_Amt(); i++ )
			{
				if ( !Q_stricmp( GetCanvasVersion( i ), pCVName ) )
				{
					bAllowLoad = true;
					UpgradeFromVersion( pCanvas, pCVName );
				}
			}
		}
		else
			UpgradeFromVersion( pCanvas, NULL );

		if ( bAllowLoad )
		{
			RestoreFullCanvas( pCanvas );
			bSuccess = true;
		}
		else if ( !bSuppressError )
		{
			vgui::PromptSimple *prompt = new vgui::PromptSimple( pEditorRoot, "ERROR" );
			prompt->MoveToCenterOfScreen();
			prompt->SetText( VarArgs( "Canvas version '%s' unsupported!", pCVName ) );
			prompt->AddButton( "Ok" );
			prompt->MakeReadyForUse();
			prompt->InvalidateLayout( true, true );
		}
	}

	UnDirtySave();

	pCanvas->deleteThis();
	return bSuccess;
}

void CNodeView::MoveNodes( Vector2D offset, KeyValues *pKV )
{
	int idx = 0;
	for (;;)
	{
		KeyValues *pNode = pKV->FindKey( VarArgs("Node_%03i", idx) );
		idx++;
		if ( !pNode )
			break;

		Vector2D pos( pNode->GetFloat( "pos_x" ), pNode->GetFloat( "pos_y" ) );
		pos += offset;
		pNode->SetFloat( "pos_x", pos.x );
		pNode->SetFloat( "pos_y", pos.y );
	}
}
void CNodeView::MoveNodes( Vector2D offset, CBaseNode *n )
{
	Vector2D pos = n->GetPosition();
	pos += offset;
	n->SetPosition( pos );
}
void CNodeView::MoveNodes( Vector2D offset, CUtlVector< CBaseNode* > &m_hList )
{
	for ( int i = 0; i < m_hList.Count(); i++ )
		MoveNodes( offset, m_hList[i] );
}
int CNodeView::GetNodeIndex( CBaseNode *n )
{
	if ( bOverrideNodeIndices )
		return m_hTmpNodeIndex.Find( n );
	return m_hNodeList.Find( n );
}
CBaseNode *CNodeView::GetNodeFromIndex( int idx )
{
	if ( bOverrideNodeIndices )
	{
		if ( !m_hTmpNodeIndex.IsValidIndex(idx) )
			return NULL;
		return m_hTmpNodeIndex[idx];
	}

	if ( !m_hNodeList.IsValidIndex(idx) )
		return NULL;
	return m_hNodeList[idx];
}
CBaseNode *CNodeView::GetNodeFromType( int type )
{
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		if ( m_hNodeList[i]->GetNodeType() == type )
			return m_hNodeList[i];
	}
	return NULL;
}

void CNodeView::CreateDragBridge( CJack *j )
{
	KillDragBridge();
	CBridge *b = j->BridgeBeginBuild();

	m_pCurBridge = b;
	m_pCurBridge->SetTemporaryTarget( GetMousePosInNodeSpace() );
}
void CNodeView::UpdateDragBridge( CJack *j, CBaseNode *n )
{
	KillDragBridge();
	m_pCurBridge = j->GetBridge( 0 );
	m_hBridgeList.FindAndRemove( m_pCurBridge );

	if ( !j->IsInput() )
		m_pCurBridge->DisconnectSource();
	else
		m_pCurBridge->DisconnectDestination();

	BeginDrag( false );

	m_pCurBridge->SetTemporaryTarget( GetMousePosInNodeSpace() );
}
void CNodeView::FinishDragBridge( CJack *j )
{
	if ( !m_pCurBridge )
		return;

	j->BridgeEndBuild( m_pCurBridge );

	m_pCurBridge = NULL;
}
void CNodeView::AddBridgeToList( CBridge *b )
{
	m_hBridgeList.AddToTail( b );
}
void CNodeView::KillDragBridge()
{
	if ( m_pCurBridge )
	{
		delete m_pCurBridge;
		m_pCurBridge = NULL;
	}
}
CJack *CNodeView::GetBridgeSnapJack()
{
	if ( !m_pCurBridge )
		return NULL;

	bool bHasInputDefined = !!m_pCurBridge->GetInputJack();
	float t = BRIDGE_DRAG_TOLERANCE;
	CJack *curJack = GetJackUnderCursor( &t, bHasInputDefined, !bHasInputDefined );
	CBaseNode *n = curJack ? curJack->GetParentNode() : NULL;

	const bool bHasVolatileBridges = curJack != NULL && curJack->IsInput() && curJack->GetNumBridges();
	bool bSameNode = n && ( m_pCurBridge->GetInputNode() == n || m_pCurBridge->GetDestinationNode() == n ) &&
					!bHasVolatileBridges;

	if ( curJack && n &&
		( curJack->IsInput() == bHasInputDefined ) &&
		!bSameNode
		)
	{
		//if ( curJack->IsInput() && curJack->GetNumBridges() )
		//	return NULL;

		CJack *potentialInput = m_pCurBridge->GetInputJack() ? m_pCurBridge->GetInputJack() : curJack;
		CJack *potentialDest = m_pCurBridge->GetDestinationJack() ? m_pCurBridge->GetDestinationJack() : curJack;
		for ( int i = 0; i < m_hBridgeList.Count(); i++ )
		{
			CBridge *b = m_hBridgeList[ i ];

			if ( potentialInput == b->GetInputJack() &&
				potentialDest == b->GetDestinationJack() )
				return NULL;
		}

		CBaseNode *origNode = bHasInputDefined ? m_pCurBridge->GetInputNode() : m_pCurBridge->GetDestinationNode();
		if ( n->RecursiveFindNode( origNode, !curJack->IsInput() ) )
			return NULL;

		if ( n == origNode )
			return NULL;

		int allowances_l, allowances_j;
		int LocalHierachy = origNode->GetHierachyTypeIterateFullyRecursive(&allowances_l);
		int JackHierachy = n->GetHierachyTypeIterateFullyRecursive(&allowances_j);

		if ( LocalHierachy == HLSLHIERACHY_MULTIPLE ||
			JackHierachy == HLSLHIERACHY_MULTIPLE )
			return NULL;

		if ( LocalHierachy != HLSLHIERACHY_NONE &&
			JackHierachy != HLSLHIERACHY_NONE &&
			JackHierachy != LocalHierachy )
			return NULL;

		if ( ( LocalHierachy > 0 && !( allowances_j & LocalHierachy ) ) ||
			( JackHierachy > 0 && !( allowances_l & JackHierachy ) ) )
			return NULL;

		if ( bHasVolatileBridges )
			curJack->GetBridge( 0 )->SetTemporaryColor( CBridge::TMPCOL_ORANGE );

		return curJack;
	}
	return NULL;
}
const bool CNodeView::ShouldDraw_Datatypes()
{
	return pEditorRoot->ShouldDraw_Datatypes();
}
const bool CNodeView::ShouldDraw_Shadows()
{
	return pEditorRoot->ShouldDraw_Shadows();
}
const bool CNodeView::ShouldDraw_Nodes()
{
	return pEditorRoot->ShouldDraw_Nodes();
}
const bool CNodeView::ShouldDraw_Jacks()
{
	return pEditorRoot->ShouldDraw_Jacks();
}
const bool CNodeView::ShouldDraw_Bridges()
{
	return pEditorRoot->ShouldDraw_Bridges();
}

void CNodeView::Paint()
{
	BaseClass::Paint();

	if ( sedit_2dprev_Enable.GetInt() )
		UpdateRTs();

	int wide, tall;
	GetSize( wide, tall );

	Paint_Grid();
	if ( ShouldDraw_Shadows() )
	{
		if ( ShouldDraw_Nodes() )
			Paint_Nodes(true);
		if ( ShouldDraw_Bridges() )
			Paint_Bridges(true);
	}
	if ( ShouldDraw_Nodes() )
		Paint_Nodes(false);
	if ( ShouldDraw_Bridges() )
		Paint_Bridges(false);

	Paint_SelectionBox();

	if ( bRenderingScreenShot )
	{
		int inset;
		inset = min( vecScreenshotBounds.z - vecScreenshotBounds.x, vecScreenshotBounds.w - vecScreenshotBounds.y );
		inset *= 0.3f;

		Vector2D top_left( vecScreenshotBounds.x, vecScreenshotBounds.w );
		Vector2D bottom_right( vecScreenshotBounds.z, vecScreenshotBounds.y );

		ToPanelSpace( top_left );
		ToPanelSpace( bottom_right );

		surface()->DrawSetTexture( m_iTex_Darken );
		surface()->DrawSetColor( Color( 0,0,0,96 ) );
		surface()->DrawTexturedSubRect( top_left.x, top_left.y, top_left.x + inset, bottom_right.y, 0, 0, 1, 1 );
		surface()->DrawTexturedSubRect( bottom_right.x - inset, top_left.y, bottom_right.x, bottom_right.y, 1, 0, 0, 1 );

		Vertex_t points[4];
		points[0].Init( Vector2D( top_left.x, top_left.y ), Vector2D( 0, 1 ) );
		points[1].Init( Vector2D( bottom_right.x, top_left.y ), Vector2D( 0, 0 ) );
		points[2].Init( Vector2D( bottom_right.x, top_left.y + inset ), Vector2D( 1, 0 ) );
		points[3].Init( Vector2D( top_left.x, top_left.y + inset ), Vector2D( 1, 1 ) );
		surface()->DrawTexturedPolygon( 4, points );

		points[0].Init( Vector2D( top_left.x, bottom_right.y - inset ), Vector2D( 1, 0 ) );
		points[1].Init( Vector2D( bottom_right.x, bottom_right.y - inset ), Vector2D( 1, 1 ) );
		points[2].Init( Vector2D( bottom_right.x, bottom_right.y ), Vector2D( 0, 1 ) );
		points[3].Init( Vector2D( top_left.x, bottom_right.y ), Vector2D( 0, 0 ) );
		surface()->DrawTexturedPolygon( 4, points );

		surface()->DrawSetTextPos( top_left.x + 15, top_left.y + 15 );
		surface()->DrawSetTextFont( pEditorRoot->GetFont1( MAX_ZOOM_FONTS - 1 ) );
		surface()->DrawSetTextColor( NODE_DRAW_COLOR_TEXT_OP );

		char name[MAX_PATH];
		if ( pEditorRoot->GetCurrentShaderName() != NULL )
			Q_FileBase( pEditorRoot->GetCurrentShaderName(), name, MAX_PATH );
		else
			Q_snprintf( name, sizeof(name), "Unnamed shader" );
		wchar_t szconverted[ 256 ];
		g_pVGuiLocalize->ConvertANSIToUnicode( name, szconverted, sizeof(szconverted)  );
		surface()->DrawPrintText( szconverted, wcslen( szconverted ) );
	}
	else
	{
		int inset = 256;

		surface()->DrawSetTexture( m_iTex_Darken );
		surface()->DrawSetColor( Color( 0,0,0,96 ) );
		surface()->DrawTexturedSubRect( 0, 0, inset, tall, 0, 0, 1, 1 );
		surface()->DrawTexturedSubRect( wide - inset, 0, wide, tall, 1, 0, 0, 1 );

		Vertex_t points[4];
		points[0].Init( Vector2D( 0, 0 ), Vector2D( 0, 1 ) );
		points[1].Init( Vector2D( wide, 0 ), Vector2D( 0, 0 ) );
		points[2].Init( Vector2D( wide, inset ), Vector2D( 1, 0 ) );
		points[3].Init( Vector2D( 0, inset ), Vector2D( 1, 1 ) );
		surface()->DrawTexturedPolygon( 4, points );

		points[0].Init( Vector2D( 0, tall - inset ), Vector2D( 1, 0 ) );
		points[1].Init( Vector2D( wide, tall - inset ), Vector2D( 1, 1 ) );
		points[2].Init( Vector2D( wide, tall ), Vector2D( 0, 1 ) );
		points[3].Init( Vector2D( 0, tall ), Vector2D( 0, 0 ) );
		surface()->DrawTexturedPolygon( 4, points );

		Paint_StatusBar();
		Paint_CompilerIcon();
		Paint_PreCompilerErrors();
	}
}
void CNodeView::Paint_StatusBar()
{
	if ( GetStackIndex() == GetCompiledStackIndex() && !m_flErrorTime )
		return;

	Vector4D col( _col_OutOfDate[0] / 255.0f,
		_col_OutOfDate[1] / 255.0f,
		_col_OutOfDate[2] / 255.0f,
		_col_OutOfDate[3] / 255.0f );

	bool bOutofDate = GetStackIndex() != GetCompiledStackIndex();
	if ( !bOutofDate )
		col.w = 0;

	if ( m_flErrorTime )
	{
		col.x = _col_Error[0] / 255.0f;
		col.y = _col_Error[1] / 255.0f;
		col.z = _col_Error[2] / 255.0f;
		col.w = abs( sin( m_flErrorTime * M_PI * 4 ) ) * m_flErrorTime;
		m_flErrorTime -= gpGlobals->frametime;
		if ( m_flErrorTime < 0 )
			m_flErrorTime = 0;
	}

	if ( col.w < 0.025f )
		return;

#define ERRORBORDER_EXTRUDE 5

	int sx,sy;
	GetSize( sx, sy );

	surface()->DrawSetColor( Color( col.x * 255, col.y * 255, col.z * 255, col.w * 255 ) );

	surface()->DrawFilledRect( 0, 0, ERRORBORDER_EXTRUDE, sy );
	surface()->DrawFilledRect( sx - ERRORBORDER_EXTRUDE, 0, sx, sy );
	surface()->DrawFilledRect( ERRORBORDER_EXTRUDE, 0, sx - ERRORBORDER_EXTRUDE, ERRORBORDER_EXTRUDE );
	surface()->DrawFilledRect( ERRORBORDER_EXTRUDE, sy - ERRORBORDER_EXTRUDE, sx - ERRORBORDER_EXTRUDE, sy );
}
void CNodeView::Paint_CompilerIcon()
{
	if ( !gShaderEditorSystem->IsCompilerRunning() )
		return;

	int sx, sy;
	GetSize( sx, sy );

	int size = scheme()->GetProportionalScaledValue( 20 );
	int offset = scheme()->GetProportionalScaledValue( 1 );

	DynRenderHelper info;

	info.Size = size * 1.5f;
	info.Orig.x = sx - size;
	info.Orig.y = size;
	info.Rotation = -45.0f * floor( 10.0f * gpGlobals->curtime );

	surface()->DrawSetTexture( m_iTex_BusyIco );
	surface()->DrawSetColor( Color( 32, 32, 32, 255 ) );
	::PaintDynamicRotationStructure( info );

	info.Orig -= Vector2D( offset, offset );
	surface()->DrawSetColor( Color( 200, 200, 200, 255 ) );
	::PaintDynamicRotationStructure( info );
}
void CNodeView::Paint_PreCompilerErrors()
{
	const int ypos_delta = 10;
	int ypos_current = 5;
	const bool bDrawAll = pEditorRoot->ShouldDraw_AllLimits();

	const Color collist[ 3 ] = {
		Color( 200, 200, 200, 255 ),	// normal
		Color( 180, 180, 60, 255 ),		// limit
		Color( 140, 50, 50, 255 ),		// error
	};

	HFont font = pEditorRoot->GetFont1( 12 );
	surface()->DrawSetTextFont( font );

	for ( int i = 0; i < hErrorList.Count(); i++ )
	{
		LimitReport_t *pE = hErrorList[i];
		const bool bIsError = pE->i_cur > pE->i_max;
		const bool bIsLimit = pE->i_cur == pE->i_max;
		if ( !bDrawAll && !bIsError )
			continue;

		int textColor = bIsError ? 2 : ( bIsLimit ? 1 : 0 );
		surface()->DrawSetTextPos( 8, ypos_current );
		ypos_current += ypos_delta;
		surface()->DrawSetTextColor( collist[ textColor ] );

		char szText[ 1024 ];
		Q_snprintf( szText, sizeof(szText), "%s: %i / %i", pE->sz_name, pE->i_cur, pE->i_max );

		wchar_t szconverted[ 1024 ];
		g_pVGuiLocalize->ConvertANSIToUnicode( szText, szconverted, sizeof(szconverted) );
		surface()->DrawPrintText( szconverted, wcslen( szconverted ) );
	}
}
void CNodeView::Paint_Grid()
{
	//const float zoomOld = m_flZoom;

	//const Vector4D gridSteps[] = {
	//	// zoom cellsize thickness colormult
	//	Vector4D( 2.0f, 4.5f, 14.0f, 0.25f ),
	//	Vector4D( 1.0f, 1.0f, 2.5f, 1.0f ),
	//};

	//for ( int i = 1; i < 2; i++ )
	{
		//m_flZoom = zoomOld * gridSteps[i].x;

		//const int flCellSize = GRID_CELL_SIZE; // * gridSteps[i].y;

		float alpha = Bias( RemapValClamped( m_flZoom, 0.5f, VIEWZOOM_OUT_MAX - 2, 1.0f, 0 ), 0.4f );
		Color bg = GetBgColor();
		//float intensity = bg.r() - 15.0f * alpha; // * gridSteps[i].w;
		Color col = Lerp( alpha, bg, _col_Grid );
		surface()->DrawSetColor( col ); //Color( intensity, intensity, intensity, 255 ) );

		int sx,sy;
		GetSize(sx,sy);

		Vector2D pos_min( 0, 0 );
		Vector2D pos_max( sx, sy );
		Vector2D tmp1,tmp2;

		ToNodeSpace( pos_min );
		ToNodeSpace( pos_max );

		Vector2D delta = pos_max - pos_min;
		delta.y *= -1.0f;

		int num_vertical = max( 1, ceil( abs(delta.x) / GRID_CELL_SIZE ) ) + 1;
		int num_horizontal = max( 1, ceil( abs(delta.y) / GRID_CELL_SIZE ) ) + 1;

		float startx = ceil( pos_min.x / GRID_CELL_SIZE ) * GRID_CELL_SIZE;
		float starty = floor( pos_min.y / GRID_CELL_SIZE ) * GRID_CELL_SIZE;

		tmp1.Init( startx, starty );
		tmp2.Init( startx+GRID_CELL_SIZE, starty-GRID_CELL_SIZE );

		ToPanelSpace( tmp1 );
		ToPanelSpace( tmp2 );

		tmp2 = tmp2 - tmp1;

		Vector2D orig( 0, 0 );
		Vector2D thickness( 2.5f, 0 );
		ToPanelSpace( orig );
		ToPanelSpace( thickness );
		thickness.x = max( 1, thickness.x - orig.x ) * 0.5f;

		tmp1 -= tmp2;

		for ( int i = 0; i < num_vertical; i++ )
		{
			surface()->DrawFilledRect( tmp1.x + tmp2.x * i - thickness.x, 0,
										tmp1.x + tmp2.x * i + thickness.x, sy );
		}
		for ( int i = 0; i < num_horizontal; i++ )
		{
			surface()->DrawFilledRect( 0, tmp1.y + tmp2.y * i - thickness.x,
										sx, tmp1.y + tmp2.y * i + thickness.x );
		}
	}

	//m_flZoom = zoomOld;
}
void CNodeView::Paint_Nodes( bool bShadow  )
{
	for ( int i = 0; i < m_hNodeList.Count(); i++ )
	{
		CBaseNode *p = m_hNodeList[i];
		p->VguiDraw( bShadow );
	}
}
void CNodeView::Paint_Bridges( bool bShadow )
{
	for ( int i = 0; i < m_hBridgeList.Count(); i++ )
	{
		CBridge *p = m_hBridgeList[i];
		p->VguiDraw( bShadow );
	}

	if ( m_pCurBridge )
	{
		m_pCurBridge->VguiDraw( bShadow );
	}
}
void CNodeView::Paint_SelectionBox()
{
	if ( !IsInDrag() )
		return;
	if ( iArmDrag != DRAG_SBOX )
		return;

	Vector2D curpos = GetMousePosInNodeSpace();
	Vector2D oldpos = m_vecSelectionBoxStart;

	ToPanelSpace( curpos );
	ToPanelSpace( oldpos );
	surface()->DrawSetColor( Color( 128, 128, 164, 128 ) );

	Vector2D _min = Vector2D( min( curpos.x, oldpos.x ),
		min( curpos.y, oldpos.y ) );
	Vector2D _max = Vector2D( max( curpos.x, oldpos.x ),
		max( curpos.y, oldpos.y ) );

	surface()->DrawFilledRect( _min.x, _min.y, _max.x, _max.y );

	surface()->DrawSetColor( Color( 48, 48, 96, 196 ) );
	surface()->DrawLine( _min.x-1, _min.y-1, _max.x, _min.y-1 );
	surface()->DrawLine( _min.x-1, _max.y, _max.x, _max.y );
	surface()->DrawLine( _min.x-1, _min.y-1, _min.x-1, _max.y );
	surface()->DrawLine( _max.x, _min.y-1, _max.x, _max.y );
}

void CNodeView::InitColors()
{
	_col_Grid = Color( 148, 148, 148, 255 );
	_col_Vignette = Color( 0, 0, 0, 96 );

	_col_OutOfDate.SetColor( 50, 50, 62, 0 );
	_col_Error.SetColor( 180, 50, 62, 0 );
}

void CNodeView::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);

	SetPaintBackgroundType( 0 );
	//SetBorder(pScheme->GetBorder("BaseBorder"));

	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);

	SetBgColor( pScheme->GetColor( "NodeView.Bg", GetBgColor() ) );

	InitColors();
	_col_Grid = pScheme->GetColor( "NodeView.Grid", _col_Grid );
	_col_Vignette = pScheme->GetColor( "NodeView.Vignette", _col_Vignette );

	_col_OutOfDate = pScheme->GetColor( "EditorRoot.OutOfDate", _col_OutOfDate );
	_col_Error = pScheme->GetColor( "EditorRoot.Error", _col_Error );
}
void CNodeView::PerformLayout()
{
	BaseClass::PerformLayout();

#if 0
	int viewer_inset = 22;
	int viewer_inset_y = 90;
	int w,t;
	//engine->GetScreenSize( w, t );
	GetParent()->GetSize( w, t );
	SetPos( viewer_inset, viewer_inset_y );
	SetSize( w - viewer_inset * 2, t - viewer_inset - viewer_inset_y );
#else
	int w,t;
	GetParent()->GetSize( w, t );
	SetPos( 0, 0 );
	SetSize( w, t );
#endif
}
HFont CNodeView::GetFontScaled( float Scale, bool &bVis, float *zoom )
{
	if ( !pEditorRoot )
	{
		bVis = true;
		return 0;
	}

	int maxFonts = MAX_ZOOM_FONTS - 1;
	Scale *= zoom ? *zoom : GetZoomScalar();
	bVis = ( Scale > 2.0f ) ? true : false;
	HFont tmp = pEditorRoot->GetFont1( clamp(((int)Scale)-1,0,maxFonts));
	return tmp;
}
HFont CNodeView::GetFontScaledNoOutline( float Scale, bool &bVis, float *zoom )
{
	return 0;
	//int maxFonts = MAX_ZOOM_FONTS - 1;
	//Scale *= zoom ? *zoom : GetZoomScalar();
	//bVis = ( Scale > 2.0f ) ? true : false;
	//HFont tmp = pEditorRoot->GetFont2(clamp(((int)Scale)-1,0,maxFonts));
	//return tmp;
}

void CNodeView::OnCommand( const char *command )
{
	BaseClass::OnCommand( command );

	if ( !Q_stricmp( command, "node_delete" ) )
		DeleteSelection();
	if ( !Q_stricmp( command, "node_disconnect" ) )
		DisconnectSelection();
	if ( !Q_stricmp( command, "node_copy" ) )
		CopySelection();
	if ( !Q_stricmp( command, "node_paste" ) )
		PasteSelection();
	if ( !Q_stricmp( command, "node_cut" ) )
	{
		CopySelection();
		DeleteSelection();
	}
}

#define NODESWITCH( nodeType, nodeClass ) case nodeType:\
	pNode = new nodeClass(this);\
	break;

CBaseNode *CNodeView::SpawnNode( int type )
{
	CBaseNode *pNode = NULL;
	switch ( type )
	{
	default:
		Warning( "node type %i not implemented!\n", type );
		break;
// SHADER STRUCTS
	case HLSLNODE_VS_IN:
		pNode = new CNodeVSInput(this);
		break;
	case HLSLNODE_VS_OUT:
		pNode = new CNodeVSOutput(this);
		break;
	case HLSLNODE_PS_IN:
		pNode = new CNodePSInput(this);
		break;
	case HLSLNODE_PS_OUT:
		pNode = new CNodePSOutput(this);
		break;

// MATRICES
	case HLSLNODE_MATRIX_MODELVIEWPROJ:
		pNode = new CNodeMatrix_MVP(this);
		break;
	case HLSLNODE_MATRIX_VIEWPROJ:
		pNode = new CNodeMatrix_VP(this);
		break;
	case HLSLNODE_MATRIX_MODEL:
		pNode = new CNodeMatrix_M(this);
		break;
	case HLSLNODE_MATRIX_VIEWMODEL:
		pNode = new CNodeMatrix_VM(this);
		break;

	NODESWITCH( HLSLNODE_MATRIX_FLASHLIGHT, CNodeMatrix_FVP );
	NODESWITCH( HLSLNODE_MATRIX_COMPOSE, CNodeMCompose );
	NODESWITCH( HLSLNODE_MATRIX_CUSTOM, CNodeMatrix_Custom );
	
// MATH
	case HLSLNODE_MATH_ADD:
		pNode = new CNodeAdd(this);
		break;
	case HLSLNODE_MATH_SUBTRACT:
		pNode = new CNodeSubtract(this);
		break;
	case HLSLNODE_MATH_MULTIPLY:
		pNode = new CNodeMultiply(this);
		break;
	case HLSLNODE_MATH_DIVIDE:
		pNode = new CNodeDivide(this);
		break;

	NODESWITCH( HLSLNODE_MATH_LERP, CNodeLerp );
	NODESWITCH( HLSLNODE_MATH_SMOOTHSTEP, CNodeSmoothstep );
	NODESWITCH( HLSLNODE_MATH_STEP, CNodeStep );
	NODESWITCH( HLSLNODE_MATH_CLAMP, CNodeClamp );
	NODESWITCH( HLSLNODE_MATH_POW, CNodePower );
	NODESWITCH( HLSLNODE_MATH_ROUND, CNodeRound );
	NODESWITCH( HLSLNODE_MATH_FRAC, CNodeFraction );
	NODESWITCH( HLSLNODE_MATH_FLOOR, CNodeFloor );
	NODESWITCH( HLSLNODE_MATH_CEIL, CNodeCeil );
	NODESWITCH( HLSLNODE_MATH_ABS, CNodeAbs );
	NODESWITCH( HLSLNODE_MATH_MIN, CNodeMin );
	NODESWITCH( HLSLNODE_MATH_MAX, CNodeMax );
	NODESWITCH( HLSLNODE_MATH_FMOD, CNodeFMod );

	NODESWITCH( HLSLNODE_MATH_SIN, CNodeSin );
	NODESWITCH( HLSLNODE_MATH_ASIN, CNodeASin );
	NODESWITCH( HLSLNODE_MATH_SINH, CNodeSinH );
	NODESWITCH( HLSLNODE_MATH_COS, CNodeCos );
	NODESWITCH( HLSLNODE_MATH_ACOS, CNodeACos );
	NODESWITCH( HLSLNODE_MATH_COSH, CNodeCosH );
	NODESWITCH( HLSLNODE_MATH_TAN, CNodeTan );
	NODESWITCH( HLSLNODE_MATH_ATAN, CNodeATan );
	NODESWITCH( HLSLNODE_MATH_TANH, CNodeTanH );
	NODESWITCH( HLSLNODE_MATH_ATAN2, CNodeATan2 );

	NODESWITCH( HLSLNODE_MATH_DEGREES, CNodeDegrees );
	NODESWITCH( HLSLNODE_MATH_RADIANS, CNodeRadians );
	NODESWITCH( HLSLNODE_MATH_SATURATE, CNodeSaturate );
	NODESWITCH( HLSLNODE_MATH_INVERT, CNodeInvert );
	NODESWITCH( HLSLNODE_MATH_SIGN, CNodeSign );
	NODESWITCH( HLSLNODE_MATH_SQRT, CNodeRoot );

	NODESWITCH( HLSLNODE_MATH_LOG, CNodeLog );
	NODESWITCH( HLSLNODE_MATH_LOG2, CNodeLog2 );
	NODESWITCH( HLSLNODE_MATH_LOG10, CNodeLog10 );
	NODESWITCH( HLSLNODE_MATH_EXP, CNodeExp );
	NODESWITCH( HLSLNODE_MATH_EXP2, CNodeExp2 );

	NODESWITCH( HLSLNODE_MATH_DDX, CNodeddx );
	NODESWITCH( HLSLNODE_MATH_DDY, CNodeddy );
	
// VECTORS
	case HLSLNODE_MATH_SWIZZLE:
		pNode = new CNodeSwizzle(this);
		break;
	case HLSLNODE_MATH_DOT:
		pNode = new CNodeDot(this);
		break;
	case HLSLNODE_MATH_NORMALIZE:
		pNode = new CNodeNormalize(this);
		break;
	case HLSLNODE_MATH_LENGTH:
		pNode = new CNodeLength(this);
		break;

	NODESWITCH( HLSLNODE_VECTOR_REFLECT, CNodeVectorReflect );
	NODESWITCH( HLSLNODE_VECTOR_REFRACT, CNodeVectorRefract );
	NODESWITCH( HLSLNODE_MATH_APPEND, CNodeAppend );
	NODESWITCH( HLSLNODE_VECTOR_CROSS, CNodeCross );
	NODESWITCH( HLSLNODE_VECTOR_DISTANCE, CNodeDistance );
	
// CONSTANTS
	case HLSLNODE_CONSTANT_LOCAL:
		pNode = new CNodeConstant(this);
		break;
	case HLSLNODE_CONSTANT_VIEW_ORIGIN:
	case HLSLNODE_CONSTANT_VIEW_FORWARD:
	case HLSLNODE_CONSTANT_VIEW_UP:
	case HLSLNODE_CONSTANT_VIEW_RIGHT:
	case HLSLNODE_CONSTANT_VIEW_WORLDSPACE_DEPTH:
	case HLSLNODE_CONSTANT_FB_PIXELSIZE:
	case HLSLNODE_CONSTANT_FOGPARAMS:
	case HLSLNODE_CONSTANT_TIME:
		{
			int targetIdx = HLSLENV_TIME;
			float defSmartVal0 = 0.0f;

			if ( type < HLSLNODE_CONSTANT_TIME )
				targetIdx = (type - HLSLNODE_CONSTANT_VIEW_ORIGIN) + HLSLENV_VIEW_ORIGIN;
			else
			{
				switch ( type )
				{
				case HLSLNODE_CONSTANT_FB_PIXELSIZE:
					targetIdx = HLSLENV_PIXEL_SIZE;
					defSmartVal0 = 1.0f;
					break;
				case HLSLNODE_CONSTANT_FOGPARAMS:
					targetIdx = HLSLENV_FOG_PARAMS;
					break;
				}
			}
			CNodeEnvC *pCastedNode = new CNodeEnvC(this,defSmartVal0);
			pCastedNode->SetEnvCType( targetIdx );
			pNode = pCastedNode;
		}
		break;
	NODESWITCH( HLSLNODE_CONSTANT_CALLBACK, CNodeCallback );
	NODESWITCH( HLSLNODE_CONSTANT_VP_MUTABLE, CNodeVmtParam_Mutable );
	NODESWITCH( HLSLNODE_CONSTANT_VP_STATIC, CNodeVmtParam_Static );
	NODESWITCH( HLSLNODE_CONSTANT_LIGHTSCALE, CNodeLightscale );
	NODESWITCH( HLSLNODE_CONSTANT_RANDOM, CNodeRandom );
	NODESWITCH( HLSLNODE_CONSTANT_BUMPBASIS, CNodeBumpBasis );
	NODESWITCH( HLSLNODE_CONSTANT_ARRAY, CNodeArray );
	NODESWITCH( HLSLNODE_CONSTANT_FLASHLIGHTPOS, CNodeFlashlight_Position );
		
// TEXTURE
	case HLSLNODE_TEXTURE_SAMPLER:
		pNode = new CNodeTexSample(this);
		break;
	NODESWITCH( HLSLNODE_TEXTURE_TRANSFORM, CNodeTexTransform );
	NODESWITCH( HLSLNODE_TEXTURE_SAMPLEROBJECT, CNodeSampler );
	NODESWITCH( HLSLNODE_TEXTURE_PARALLAX, CNodeParallax );
	NODESWITCH( HLSLNODE_TEXTURE_PARALLAX_SHADOW, CNodeParallax_StdShadow );
	
// CONTROLFLOW
	case HLSLNODE_CONTROLFLOW_LOOP:
		pNode = new CNodeLoop(this);
		break;
	NODESWITCH( HLSLNODE_CONTROLFLOW_COMBO, CNodeCombo );
	NODESWITCH( HLSLNODE_CONTROLFLOW_CONDITION, CNodeCondition );
	NODESWITCH( HLSLNODE_CONTROLFLOW_BREAK, CNodeBreak );
	NODESWITCH( HLSLNODE_CONTROLFLOW_CLIP, CNodeClip );
	
// UTILITY
	case HLSLNODE_UTILITY_DECLARE:
		pNode = new CNodeUtility_Declare(this);
		break;
	case HLSLNODE_UTILITY_ASSIGN:
		pNode = new CNodeUtility_Assign(this);
		break;

	NODESWITCH( HLSLNODE_STUDIO_VERTEXLIGHTING, CNodeStdVLight );
	NODESWITCH( HLSLNODE_STUDIO_PIXELSHADER_LIGHTING, CNodeStdPLight );
	NODESWITCH( HLSLNODE_STUDIO_PIXELSHADER_SPECLIGHTING, CNodeStdPSpecLight );
	NODESWITCH( HLSLNODE_STUDIO_SKINNING, CNodeStdSkinning );
	NODESWITCH( HLSLNODE_STUDIO_MORPH, CNodeStdMorph );
	NODESWITCH( HLSLNODE_STUDIO_VCOMPRESSION, CNodeVCompression );
	NODESWITCH( HLSLNODE_UTILITY_FOG, CNodeFog );
	NODESWITCH( HLSLNODE_UTILITY_FINAL, CNodeFinal );
	NODESWITCH( HLSLNODE_UTILITY_FLASHLIGHT, CNodeFlashlight );
	NODESWITCH( HLSLNODE_UTILITY_CUSTOMCODE, CNodeCustom );

// POST PROCESSING EFFECTS
	NODESWITCH( HLSLNODE_POSTPROCESSING_INPUT, CNodePP_Input );
	NODESWITCH( HLSLNODE_POSTPROCESSING_OUTPUT, CNodePP_Output );

	NODESWITCH( HLSLNODE_POSTPROCESSING_CREATE_RT, CNodePP_RT );
	NODESWITCH( HLSLNODE_POSTPROCESSING_CREATE_MAT, CNodePP_Mat );
	NODESWITCH( HLSLNODE_POSTPROCESSING_COPY_RT, CNodePP_CopyRT );
	
	NODESWITCH( HLSLNODE_POSTPROCESSING_PUSH_VP, CNodePP_VP_Push );
	NODESWITCH( HLSLNODE_POSTPROCESSING_POP_VP, CNodePP_VP_Pop );
	NODESWITCH( HLSLNODE_POSTPROCESSING_SET_RT, CNodePP_VP_SetRT );

	NODESWITCH( HLSLNODE_POSTPROCESSING_CLEAR_BUFFERS, CNodePP_ClearBuffers );
	NODESWITCH( HLSLNODE_POSTPROCESSING_DRAW_MATERIAL, CNodePP_DrawMat );
	NODESWITCH( HLSLNODE_POSTPROCESSING_UPDATE_FB, CNodePP_UpdateFB );
	NODESWITCH( HLSLNODE_POSTPROCESSING_RENDER_VIEW, CNodePP_RenderView );

// MISC
	NODESWITCH( HLSLNODE_OTHER_COMMENT, CNodeComment );
	}

	if ( !pNode )
		return NULL;

	if ( !( GetFlowgraphType() & pNode->GetAllowedFlowgraphTypes() ) )
	{
		delete pNode;
		return NULL;
	}

	pNode->Spawn();

	OnHierachyChanged();
	m_hNodeList.AddToTail( pNode );
	return pNode;
}
void CNodeView::OnSpawnNode( int type )
{
	CBaseNode *pNode = SpawnNode( type );
	if ( !pNode )
		return;

	pNode->SetPosition( m_vecMenuCreationPosition, true );
}

void CNodeView::OnMenuClosed( KeyValues *pData )
{
	Panel *pMenu = ((Panel*)pData->GetPtr( "panel" ));
	if ( pMenu == m_ContextMenu.Get() )
	{
		delete m_ContextMenu.Get();
		m_ContextMenu = NULL;
	}
}
void CNodeView::CreateContextMenu( CBaseNode *pMouseOverNode )
{
	if ( m_ContextMenu.Get() )
	{
		delete m_ContextMenu.Get();
		m_ContextMenu = NULL;
	}

	UpdateMousePos();
	m_vecMenuCreationPosition.Init( Mx, My );
	ToNodeSpace( m_vecMenuCreationPosition );

	bool bMouseOverNode = pMouseOverNode != NULL;
	bool bOtherNodesSelected = IsSelectionBiggerThanOne();
	bool bAnyNodesSelected = bMouseOverNode || bOtherNodesSelected;

	bool bCrucialNode = bMouseOverNode && pMouseOverNode->IsNodeCrucial();
	bool bBlockOnSingleCrucialNode = bCrucialNode && !bOtherNodesSelected;
	bool bNodesInCopySlot = !!pEditorRoot->GetCopyNodes();

	Panel *pMenuParent = this;

	Menu *m = new Menu( pMenuParent, "contextmenu" );
	Menu *padd = new Menu( m, "addmenu" );

	AddNodesToContextMenu( padd );

	m->AddCascadingMenuItem( "New node", this, padd );

	int tmp;
	tmp = m->AddMenuItem( "Copy", "node_copy", this );
	if ( !bAnyNodesSelected || bBlockOnSingleCrucialNode )
		m->GetMenuItem( tmp )->SetEnabled( false );

	tmp = m->AddMenuItem( "Cut", "node_cut", this );
	if ( !bAnyNodesSelected || bBlockOnSingleCrucialNode )
		m->GetMenuItem( tmp )->SetEnabled( false );

	tmp = m->AddMenuItem( "Paste", "node_paste", this );
	if ( !bNodesInCopySlot )
		m->GetMenuItem( tmp )->SetEnabled( false );

	tmp = m->AddMenuItem( "Delete", "node_delete", this );
	if ( !bAnyNodesSelected || bBlockOnSingleCrucialNode )
		m->GetMenuItem( tmp )->SetEnabled( false );

	tmp = m->AddMenuItem( "Disconnect", "node_disconnect", this );
	if ( !bAnyNodesSelected )
		m->GetMenuItem( tmp )->SetEnabled( false );

	KeyValues *pKV_P = new KeyValues("OpenProperties");
	pKV_P->SetPtr( "pNode", pMouseOverNode );
	tmp = m->AddMenuItem( "Properties", pKV_P, this );
	if ( ( bOtherNodesSelected || !bAnyNodesSelected ) || !bMouseOverNode ) m->GetMenuItem( tmp )->SetEnabled( false );

	m_ContextMenu = m;
	//input()->SetMouseFocus( m_ContextMenu->GetVPanel() );

	Menu::PlaceContextMenu( pMenuParent, m_ContextMenu );
}
void CNodeView::AddNodesToContextMenu( Menu *pNodeMenu )
{
	FlowGraphType_t type = GetFlowgraphType();

	Menu *padd_misc = new Menu( pNodeMenu, "addmenu_misc" );

	if ( type == FLOWGRAPH_HLSL || type == FLOWGRAPH_HLSL_TEMPLATE )
	{
		Menu *padd_math = new Menu( pNodeMenu, "addmenu_math" );
		Menu *padd_math_round = new Menu( pNodeMenu, "addmenu_math_round" );
		Menu *padd_math_trigo = new Menu( pNodeMenu, "addmenu_math_trigo" );
		Menu *padd_math_vec = new Menu( pNodeMenu, "addmenu_math_vec" );
		Menu *padd_const = new Menu( pNodeMenu, "addmenu_constants" );
		Menu *padd_matrix = new Menu( pNodeMenu, "addmenu_matrix" );
		Menu *padd_tex = new Menu( pNodeMenu, "addmenu_textures" );
		Menu *padd_controlflow = new Menu( pNodeMenu, "addmenu_controlflow" );
		Menu *padd_utility = new Menu( pNodeMenu, "addmenu_utility" );
		Menu *padd_utility_studio = new Menu( pNodeMenu, "addmenu_utility_studio" );

		padd_math->AddMenuItem( "Add", new KeyValues("spawnNode","type",HLSLNODE_MATH_ADD), this );
		padd_math->AddMenuItem( "Subtract", new KeyValues("spawnNode","type",HLSLNODE_MATH_SUBTRACT), this );
		padd_math->AddMenuItem( "Multiply", new KeyValues("spawnNode","type",HLSLNODE_MATH_MULTIPLY), this );
		padd_math->AddMenuItem( "Divide", new KeyValues("spawnNode","type",HLSLNODE_MATH_DIVIDE), this );
		padd_math->AddMenuItem( "Root", new KeyValues("spawnNode","type",HLSLNODE_MATH_SQRT), this );
		padd_math->AddMenuItem( "Invert", new KeyValues("spawnNode","type",HLSLNODE_MATH_INVERT), this );
		padd_math->AddMenuItem( "Power", new KeyValues("spawnNode","type",HLSLNODE_MATH_POW), this );
		padd_math->AddMenuItem( "Lerp", new KeyValues("spawnNode","type",HLSLNODE_MATH_LERP), this );
		padd_math->AddMenuItem( "FMod", new KeyValues("spawnNode","type",HLSLNODE_MATH_FMOD), this );
		padd_math->AddMenuItem( "Degrees", new KeyValues("spawnNode","type",HLSLNODE_MATH_DEGREES), this );
		padd_math->AddMenuItem( "Radians", new KeyValues("spawnNode","type",HLSLNODE_MATH_RADIANS), this );
		padd_math->AddMenuItem( "Log", new KeyValues("spawnNode","type",HLSLNODE_MATH_LOG), this );
		padd_math->AddMenuItem( "Log2", new KeyValues("spawnNode","type",HLSLNODE_MATH_LOG2), this );
		padd_math->AddMenuItem( "Log10", new KeyValues("spawnNode","type",HLSLNODE_MATH_LOG10), this );
		padd_math->AddMenuItem( "Exp", new KeyValues("spawnNode","type",HLSLNODE_MATH_EXP), this );
		padd_math->AddMenuItem( "Exp2", new KeyValues("spawnNode","type",HLSLNODE_MATH_EXP2), this );
		padd_math->AddMenuItem( "ddx", new KeyValues("spawnNode","type",HLSLNODE_MATH_DDX), this );
		padd_math->AddMenuItem( "ddy", new KeyValues("spawnNode","type",HLSLNODE_MATH_DDY), this );
		pNodeMenu->AddCascadingMenuItem( "Math", this, padd_math );
	
		padd_math_round->AddMenuItem( "Abs", new KeyValues("spawnNode","type",HLSLNODE_MATH_ABS), this );
		padd_math_round->AddMenuItem( "Min", new KeyValues("spawnNode","type",HLSLNODE_MATH_MIN), this );
		padd_math_round->AddMenuItem( "Max", new KeyValues("spawnNode","type",HLSLNODE_MATH_MAX), this );
		padd_math_round->AddMenuItem( "Smooth step", new KeyValues("spawnNode","type",HLSLNODE_MATH_SMOOTHSTEP), this );
		padd_math_round->AddMenuItem( "Step", new KeyValues("spawnNode","type",HLSLNODE_MATH_STEP), this );
		padd_math_round->AddMenuItem( "Sign", new KeyValues("spawnNode","type",HLSLNODE_MATH_SIGN), this );
		padd_math_round->AddMenuItem( "Clamp", new KeyValues("spawnNode","type",HLSLNODE_MATH_CLAMP), this );
		padd_math_round->AddMenuItem( "Saturate", new KeyValues("spawnNode","type",HLSLNODE_MATH_SATURATE), this );
		padd_math_round->AddMenuItem( "Round", new KeyValues("spawnNode","type",HLSLNODE_MATH_ROUND), this );
		padd_math_round->AddMenuItem( "Fraction", new KeyValues("spawnNode","type",HLSLNODE_MATH_FRAC), this );
		padd_math_round->AddMenuItem( "Floor", new KeyValues("spawnNode","type",HLSLNODE_MATH_FLOOR), this );
		padd_math_round->AddMenuItem( "Ceil", new KeyValues("spawnNode","type",HLSLNODE_MATH_CEIL), this );
		pNodeMenu->AddCascadingMenuItem( "Math (Rounding)", this, padd_math_round );

		padd_math_vec->AddMenuItem( "Dot", new KeyValues("spawnNode","type",HLSLNODE_MATH_DOT), this );
		padd_math_vec->AddMenuItem( "Length", new KeyValues("spawnNode","type",HLSLNODE_MATH_LENGTH), this );
		padd_math_vec->AddMenuItem( "Normalize", new KeyValues("spawnNode","type",HLSLNODE_MATH_NORMALIZE), this );
		padd_math_vec->AddMenuItem( "Append", new KeyValues("spawnNode","type",HLSLNODE_MATH_APPEND), this );
		padd_math_vec->AddMenuItem( "Swizzle", new KeyValues("spawnNode","type",HLSLNODE_MATH_SWIZZLE), this );
		padd_math_vec->AddMenuItem( "Reflect", new KeyValues("spawnNode","type",HLSLNODE_VECTOR_REFLECT), this );
		padd_math_vec->AddMenuItem( "Refract", new KeyValues("spawnNode","type",HLSLNODE_VECTOR_REFRACT), this );
		padd_math_vec->AddMenuItem( "Crossproduct", new KeyValues("spawnNode","type",HLSLNODE_VECTOR_CROSS), this );
		padd_math_vec->AddMenuItem( "Distance", new KeyValues("spawnNode","type",HLSLNODE_VECTOR_DISTANCE), this );
		pNodeMenu->AddCascadingMenuItem( "Math (Vectors)", this, padd_math_vec );

		padd_math_trigo->AddMenuItem( "Sin", new KeyValues("spawnNode","type",HLSLNODE_MATH_SIN), this );
		padd_math_trigo->AddMenuItem( "ASin", new KeyValues("spawnNode","type",HLSLNODE_MATH_ASIN), this );
		padd_math_trigo->AddMenuItem( "SinH", new KeyValues("spawnNode","type",HLSLNODE_MATH_SINH), this );
		padd_math_trigo->AddMenuItem( "Cos", new KeyValues("spawnNode","type",HLSLNODE_MATH_COS), this );
		padd_math_trigo->AddMenuItem( "ACos", new KeyValues("spawnNode","type",HLSLNODE_MATH_ACOS), this );
		padd_math_trigo->AddMenuItem( "CosH", new KeyValues("spawnNode","type",HLSLNODE_MATH_COSH), this );
		padd_math_trigo->AddMenuItem( "Tan", new KeyValues("spawnNode","type",HLSLNODE_MATH_TAN), this );
		padd_math_trigo->AddMenuItem( "ATan", new KeyValues("spawnNode","type",HLSLNODE_MATH_ATAN), this );
		padd_math_trigo->AddMenuItem( "TanH", new KeyValues("spawnNode","type",HLSLNODE_MATH_TANH), this );
		padd_math_trigo->AddMenuItem( "ATan2", new KeyValues("spawnNode","type",HLSLNODE_MATH_ATAN2), this );
		pNodeMenu->AddCascadingMenuItem( "Math (Trig)", this, padd_math_trigo );

		padd_const->AddMenuItem( "Local constant", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_LOCAL), this );
		padd_const->AddMenuItem( "Callback constant", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_CALLBACK), this );
		padd_const->AddMenuItem( "VParam static", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_VP_STATIC), this );
		padd_const->AddMenuItem( "VParam mutable", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_VP_MUTABLE), this );
		padd_const->AddMenuItem( "Random float", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_RANDOM), this );
		padd_const->AddMenuItem( "Array", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_ARRAY), this );
		padd_const->AddMenuItem( "View origin", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_VIEW_ORIGIN), this );
		padd_const->AddMenuItem( "View forward", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_VIEW_FORWARD), this );
		padd_const->AddMenuItem( "View up", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_VIEW_UP), this );
		padd_const->AddMenuItem( "View right", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_VIEW_RIGHT), this );
		padd_const->AddMenuItem( "View world distance", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_VIEW_WORLDSPACE_DEPTH), this );
		padd_const->AddMenuItem( "Time", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_TIME), this );
		padd_const->AddMenuItem( "FB texelsize", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_FB_PIXELSIZE), this );
		padd_const->AddMenuItem( "Fog params", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_FOGPARAMS), this );
		padd_const->AddMenuItem( "Lightscale", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_LIGHTSCALE), this );
		padd_const->AddMenuItem( "Bump basis", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_BUMPBASIS), this );
		padd_const->AddMenuItem( "Flashlight pos", new KeyValues("spawnNode","type",HLSLNODE_CONSTANT_FLASHLIGHTPOS), this );
		pNodeMenu->AddCascadingMenuItem( "Constants", this, padd_const );
	
		padd_matrix->AddMenuItem( "Matrix - ModelViewProj", new KeyValues("spawnNode","type",HLSLNODE_MATRIX_MODELVIEWPROJ), this );
		padd_matrix->AddMenuItem( "Matrix - ViewProj", new KeyValues("spawnNode","type",HLSLNODE_MATRIX_VIEWPROJ), this );
		padd_matrix->AddMenuItem( "Matrix - Model", new KeyValues("spawnNode","type",HLSLNODE_MATRIX_MODEL), this );
		padd_matrix->AddMenuItem( "Matrix - ViewModel", new KeyValues("spawnNode","type",HLSLNODE_MATRIX_VIEWMODEL), this );
		padd_matrix->AddMenuItem( "Matrix - Flashlight VP", new KeyValues("spawnNode","type",HLSLNODE_MATRIX_FLASHLIGHT), this );
		padd_matrix->AddMenuItem( "Matrix compose", new KeyValues("spawnNode","type",HLSLNODE_MATRIX_COMPOSE), this );
		padd_matrix->AddMenuItem( "User matrix", new KeyValues("spawnNode","type",HLSLNODE_MATRIX_CUSTOM), this );
		pNodeMenu->AddCascadingMenuItem( "Matrices", this, padd_matrix );
	
		padd_tex->AddMenuItem( "Texture sample", new KeyValues("spawnNode","type",HLSLNODE_TEXTURE_SAMPLER), this );
		padd_tex->AddMenuItem( "Texture transform", new KeyValues("spawnNode","type",HLSLNODE_TEXTURE_TRANSFORM), this );
		padd_tex->AddMenuItem( "Sampler object", new KeyValues("spawnNode","type",HLSLNODE_TEXTURE_SAMPLEROBJECT), this );
		padd_tex->AddMenuItem( "Parallax UV", new KeyValues("spawnNode","type",HLSLNODE_TEXTURE_PARALLAX), this );
		padd_tex->AddMenuItem( "Parallax shadow", new KeyValues("spawnNode","type",HLSLNODE_TEXTURE_PARALLAX_SHADOW), this );
		pNodeMenu->AddCascadingMenuItem( "Textures", this, padd_tex );
	
		padd_controlflow->AddMenuItem( "Loop", new KeyValues("spawnNode","type",HLSLNODE_CONTROLFLOW_LOOP), this );
		//padd_controlflow->AddMenuItem( "Break", new KeyValues("spawnNode","type",HLSLNODE_CONTROLFLOW_BREAK), this );
		padd_controlflow->AddMenuItem( "Condition", new KeyValues("spawnNode","type",HLSLNODE_CONTROLFLOW_CONDITION), this );
		padd_controlflow->AddMenuItem( "Shader combo", new KeyValues("spawnNode","type",HLSLNODE_CONTROLFLOW_COMBO), this );
		padd_controlflow->AddMenuItem( "Clip", new KeyValues("spawnNode","type",HLSLNODE_CONTROLFLOW_CLIP), this );
		pNodeMenu->AddCascadingMenuItem( "Control flow", this, padd_controlflow );

		padd_utility->AddMenuItem( "Declare", new KeyValues("spawnNode","type",HLSLNODE_UTILITY_DECLARE), this );
		padd_utility->AddMenuItem( "Assign", new KeyValues("spawnNode","type",HLSLNODE_UTILITY_ASSIGN), this );
		padd_utility->AddMenuItem( "Custom code", new KeyValues("spawnNode","type",HLSLNODE_UTILITY_CUSTOMCODE), this );
		padd_utility->AddMenuItem( "Pixel fog", new KeyValues("spawnNode","type",HLSLNODE_UTILITY_FOG), this );
		padd_utility->AddMenuItem( "Final output", new KeyValues("spawnNode","type",HLSLNODE_UTILITY_FINAL), this );
		padd_utility->AddMenuItem( "Flashlight", new KeyValues("spawnNode","type",HLSLNODE_UTILITY_FLASHLIGHT), this );

		padd_utility_studio->AddMenuItem( "Vertex lighting", new KeyValues("spawnNode","type",HLSLNODE_STUDIO_VERTEXLIGHTING), this );
		padd_utility_studio->AddMenuItem( "Pixelshader lighting", new KeyValues("spawnNode","type",HLSLNODE_STUDIO_PIXELSHADER_LIGHTING), this );
		padd_utility_studio->AddMenuItem( "Pixelshader specular", new KeyValues("spawnNode","type",HLSLNODE_STUDIO_PIXELSHADER_SPECLIGHTING), this );
		padd_utility_studio->AddMenuItem( "Skinning", new KeyValues("spawnNode","type",HLSLNODE_STUDIO_SKINNING), this );
		padd_utility_studio->AddMenuItem( "Morph", new KeyValues("spawnNode","type",HLSLNODE_STUDIO_MORPH), this );
		padd_utility_studio->AddMenuItem( "Vertex decompress", new KeyValues("spawnNode","type",HLSLNODE_STUDIO_VCOMPRESSION), this );
		padd_utility->AddCascadingMenuItem( "Studio model", this, padd_utility_studio );
		pNodeMenu->AddCascadingMenuItem( "Utility", this, padd_utility );
	}
	else if ( type == FLOWGRAPH_POSTPROC )
	{
		Menu *padd_render = new Menu( pNodeMenu, "addmenu_render" );
		Menu *padd_stack = new Menu( pNodeMenu, "addmenu_stack" );
		Menu *padd_resource = new Menu( pNodeMenu, "addmenu_resource" );

		padd_render->AddMenuItem( "Draw material", new KeyValues("spawnNode","type",HLSLNODE_POSTPROCESSING_DRAW_MATERIAL), this );
		padd_render->AddMenuItem( "Clear buffers", new KeyValues("spawnNode","type",HLSLNODE_POSTPROCESSING_CLEAR_BUFFERS), this );
		padd_render->AddMenuItem( "Update framebuffer texture", new KeyValues("spawnNode","type",HLSLNODE_POSTPROCESSING_UPDATE_FB), this );
		padd_render->AddMenuItem( "Render view", new KeyValues("spawnNode","type",HLSLNODE_POSTPROCESSING_RENDER_VIEW), this );
		pNodeMenu->AddCascadingMenuItem( "Render", this, padd_render );

		padd_stack->AddMenuItem( "Push viewport", new KeyValues("spawnNode","type",HLSLNODE_POSTPROCESSING_PUSH_VP), this );
		padd_stack->AddMenuItem( "Pop viewport", new KeyValues("spawnNode","type",HLSLNODE_POSTPROCESSING_POP_VP), this );
		padd_stack->AddMenuItem( "Set rendertarget", new KeyValues("spawnNode","type",HLSLNODE_POSTPROCESSING_SET_RT), this );
		pNodeMenu->AddCascadingMenuItem( "Viewport stack", this, padd_stack );

		padd_resource->AddMenuItem( "Rendertarget", new KeyValues("spawnNode","type",HLSLNODE_POSTPROCESSING_CREATE_RT), this );
		padd_resource->AddMenuItem( "Material", new KeyValues("spawnNode","type",HLSLNODE_POSTPROCESSING_CREATE_MAT), this );
		padd_resource->AddMenuItem( "Copy rendertarget", new KeyValues("spawnNode","type",HLSLNODE_POSTPROCESSING_COPY_RT), this );
		pNodeMenu->AddCascadingMenuItem( "Resource", this, padd_resource );
	}

	padd_misc->AddMenuItem( "Group", new KeyValues("spawnNode","type",HLSLNODE_OTHER_COMMENT), this );
	pNodeMenu->AddCascadingMenuItem( "Misc", this, padd_misc );
}
void CNodeView::OnOpenProperties( KeyValues *data )
{
	CBaseNode *n = static_cast< CBaseNode* >( data->GetPtr( "pNode" ) );
	if ( !n )
		return;
	CreatePropertyDialog( n );
}
void CNodeView::CreatePropertyDialog( CBaseNode *pNode )
{
	m_Properties = new CNodePropertySheet( pNode, this, "" );
	m_Properties->MakeReadyForUse();
	PropertySheet *pSheet = m_Properties->GetPropertySheet();
	pSheet->MakeReadyForUse();
	pSheet->InvalidateLayout( true, false );
	KeyValues *pKV = m_Properties->GetPropertyContainer();

	int type = pNode->GetNodeType();
	CUtlVector< CSheet_Base* > hSheets;

	switch ( type )
	{
// SEMANTICS
	case HLSLNODE_VS_IN:
			hSheets.AddToTail( new CSheet_VSInput( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_VS_OUT:
	case HLSLNODE_PS_IN:
			hSheets.AddToTail( new CSheet_PSInVSOut( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_PS_OUT:
			hSheets.AddToTail( new CSheet_PSOutput( pNode, this, pKV, pSheet ) );
		break;

// MATH
	case HLSLNODE_MATH_MULTIPLY:
			hSheets.AddToTail( new CSheet_Multiply( pNode, this, pKV, pSheet ) );
		break;

// VECTORS
	case HLSLNODE_MATH_SWIZZLE:
			hSheets.AddToTail( new CSheet_Swizzle( pNode, this, pKV, pSheet ) );
		break;

// CONSTANTS
	case HLSLNODE_CONSTANT_LOCAL:
			hSheets.AddToTail( new CSheet_Constant( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_CONSTANT_CALLBACK:
			hSheets.AddToTail( new CSheet_Callback( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_CONSTANT_VP_MUTABLE:
			hSheets.AddToTail( new CSheet_VParam_Mutable( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_CONSTANT_VP_STATIC:
			hSheets.AddToTail( new CSheet_VParam_Static( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_CONSTANT_LIGHTSCALE:
			hSheets.AddToTail( new CSheet_Lightscale( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_CONSTANT_RANDOM:
			hSheets.AddToTail( new CSheet_Random( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_CONSTANT_ARRAY:
			hSheets.AddToTail( new CSheet_Array( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_CONSTANT_FB_PIXELSIZE:
			hSheets.AddToTail( new CSheet_EnvCTexelsize( pNode, this, pKV, pSheet ) );
		break;

// TEXTURES
	case HLSLNODE_TEXTURE_SAMPLER:
			hSheets.AddToTail( new CSheet_TextureSample( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_TEXTURE_TRANSFORM:
			hSheets.AddToTail( new CSheet_TexTransform( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_TEXTURE_SAMPLEROBJECT:
		{
			CSheet_TextureSample *pAddSheet = new CSheet_TextureSample( pNode, this, pKV, pSheet );
			pAddSheet->MakeSamplerOnly();
			hSheets.AddToTail( pAddSheet );
		}
		break;
	case HLSLNODE_TEXTURE_PARALLAX:
			hSheets.AddToTail( new CSheet_Parallax( pNode, this, pKV, pSheet ) );
		break;
		
// FLOW CONTROL
	case HLSLNODE_CONTROLFLOW_LOOP:
			hSheets.AddToTail( new CSheet_Loop( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_CONTROLFLOW_CONDITION:
			hSheets.AddToTail( new CSheet_Condition( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_CONTROLFLOW_COMBO:
			hSheets.AddToTail( new CSheet_Combo( pNode, this, pKV, pSheet ) );
		break;

// UTILITY
	case HLSLNODE_STUDIO_VERTEXLIGHTING:
			hSheets.AddToTail( new CSheet_Std_VLight( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_STUDIO_PIXELSHADER_LIGHTING:
			hSheets.AddToTail( new CSheet_Std_PLight( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_STUDIO_PIXELSHADER_SPECLIGHTING:
			hSheets.AddToTail( new CSheet_Std_PSpecLight( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_STUDIO_SKINNING:
			hSheets.AddToTail( new CSheet_Std_Skinning( pNode, this, pKV, pSheet ) );
			hSheets.Tail()->SetDynamicTitle( "Skinning" );
		break;
	case HLSLNODE_STUDIO_MORPH:
			hSheets.AddToTail( new CSheet_Std_Skinning( pNode, this, pKV, pSheet ) );
			hSheets.Tail()->SetDynamicTitle( "Morphing" );
		break;
	case HLSLNODE_STUDIO_VCOMPRESSION:
			hSheets.AddToTail( new CSheet_VCompression( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_UTILITY_FINAL:
			hSheets.AddToTail( new CSheet_Final( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_UTILITY_FLASHLIGHT:
			hSheets.AddToTail( new CSheet_Flashlight( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_UTILITY_CUSTOMCODE:
		{
			CSheet_Custom_IO *pIO = new CSheet_Custom_IO( pNode, this, pKV, pSheet );
			CSheet_Custom *pCode = new CSheet_Custom( pNode, this, pKV, pSheet );
			pCode->SetIOPage( pIO );
			hSheets.AddToTail( pCode );
			hSheets.AddToTail( pIO );
		}
		break;

// MATRICES
	case HLSLNODE_MATRIX_COMPOSE:
			hSheets.AddToTail( new CSheet_MComp( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_MATRIX_CUSTOM:
			hSheets.AddToTail( new CSheet_CMatrix( pNode, this, pKV, pSheet ) );
		break;

// POST PROCESS
	case HLSLNODE_POSTPROCESSING_CREATE_RT:
			hSheets.AddToTail( new CSheet_PP_RT( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_POSTPROCESSING_CREATE_MAT:
			hSheets.AddToTail( new CSheet_PP_Mat( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_POSTPROCESSING_CLEAR_BUFFERS:
			hSheets.AddToTail( new CSheet_PP_ClearBuff( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_POSTPROCESSING_DRAW_MATERIAL:
			hSheets.AddToTail( new CSheet_PP_DrawMat( pNode, this, pKV, pSheet ) );
		break;
	case HLSLNODE_POSTPROCESSING_RENDER_VIEW:
			hSheets.AddToTail( new CSheet_PP_RenderView( pNode, this, pKV, pSheet ) );
		break;

// MISC
	case HLSLNODE_OTHER_COMMENT:
			hSheets.AddToTail( new CSheet_Comment( pNode, this, pKV, pSheet ) );
		break;
	}

	if ( !pNode->IsNodeCrucial() )
	{
		hSheets.AddToTail( new CSheet_General( pNode, this, pKV, pSheet ) );
	}

	for ( int i = 0; i < hSheets.Count(); i++ )
	{
		m_Properties->AddPage( hSheets[i], hSheets[i]->GetSheetTitle() );
	}

	hSheets.Purge();
	m_Properties->DoModal();
	m_Properties->SetDeleteSelfOnClose( true );
}
void CNodeView::MakeHistoryDirty()
{
	m_bHistoryDirty = true;
}
void CNodeView::Think_UpdateHistory()
{
	if ( !m_bHistoryDirty )
		return;

	m_bHistoryDirty = false;
	if ( IsInDrag() )
		return;

	HistoryAction( HACT_PUSH );
}
void CNodeView::HistoryAction( HistoryActions_t action )
{
#if ( ENABLE_HISTORY == 0 )
	return;
#endif

	switch ( action )
	{
	case HACT_UNDO:
		{
			if ( m_hNodeViewHistory.Count() < 2 )
				break;

			m_HistoryIndex++;

			ApplyHistoryEntry( m_hNodeViewHistory[ 1 ] );
			m_hNodeViewHistory_Redo.AddToHead( m_hNodeViewHistory[ 0 ] );
			m_hNodeViewHistory.Remove( 0 );

			Think_UpdateSolvers();
		}
		break;
	case HACT_REDO:
		{
			if ( m_hNodeViewHistory_Redo.Count() < 1 )
				break;

			m_HistoryIndex++;

			ApplyHistoryEntry( m_hNodeViewHistory_Redo[ 0 ] );
			m_hNodeViewHistory.AddToHead( m_hNodeViewHistory_Redo[ 0 ] );
			m_hNodeViewHistory_Redo.Remove( 0 );

			Think_UpdateSolvers();
		}
		break;
	case HACT_PUSH:
		{
			NHistoryEntry_t *pE = AllocHistoryEntry();
			if ( !pE )
				break;

			m_HistoryIndex++;

			m_hNodeViewHistory.AddToHead( pE );
			FlushHistory( false, true );

			while ( m_hNodeViewHistory.Count() > NODEVIEWHISTORY_MAXUNDO )
			{
				delete m_hNodeViewHistory[ NODEVIEWHISTORY_MAXUNDO ];
				m_hNodeViewHistory.Remove( NODEVIEWHISTORY_MAXUNDO );
			}
		}
		break;
	}

	m_bHistoryDirty = false;
}
void CNodeView::ApplyHistoryEntry( NHistoryEntry_t *pEntry )
{
	RestoreFullCanvas( pEntry->pKV );
	pEditorRoot->SetCurrentShaderName( pEntry->szShaderName );
}
NHistoryEntry_t *CNodeView::AllocHistoryEntry()
{
	KeyValues *pCanvas = BuildFullCanvas();
	if ( !pCanvas )
		return NULL;

	const char *pszSName = pEditorRoot->GetCurrentShaderName();
	NHistoryEntry_t *pE = new NHistoryEntry_t( pCanvas, pszSName );
	return pE;
}
void CNodeView::FlushHistory( bool bPast, bool bFuture )
{
	if ( bPast )
		m_hNodeViewHistory.PurgeAndDeleteElements();

	if ( bFuture )
		m_hNodeViewHistory_Redo.PurgeAndDeleteElements();
}
int CNodeView::GetNumHistoryEntires_Undo()
{
	return m_hNodeViewHistory.Count();
}
int CNodeView::GetNumHistoryEntires_Redo()
{
	return m_hNodeViewHistory_Redo.Count();
}