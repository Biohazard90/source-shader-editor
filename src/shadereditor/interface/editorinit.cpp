
#include "cbase.h"
#include "vgui/isystem.h"

void EngineCopy( const char *a, const char *b )
{
#ifdef SHADER_EDITOR_DLL_2013
	engine->CopyLocalFile( a, b );
#else
	engine->CopyFile( a, b );
#endif
}

float GetEditorTime()
{
	return g_pVGuiSystem->GetCurrentTime();
}

#include <windows.h>
#include "EditorInit.h"
#include "editorcommon.h"

#include <tlhelp32.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <direct.h>

#include "view_shared.h"
#include "materialsystem/imesh.h"

void ForceTerminateCompilers()
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == 1)
	{
		while (Process32Next(snapshot, &entry) == 1)
		{
			if (stricmp(entry.szExeFile, "shadercompile.exe") == 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

				TerminateProcess( hProcess, 0 );

				CloseHandle(hProcess);
			}
		}
	}

	CloseHandle(snapshot);
}


#ifdef SHADER_EDITOR_DLL_SWARM
void VCRHook_WaitForSingleObject(
	__in HANDLE hHandle,
	__in DWORD dwMilliseconds
	)
{
	WaitForSingleObject( hHandle, dwMilliseconds );
}
#endif


ShaderEditorInterface gShaderEditorInterface;
ShaderEditorInterface *shaderEdit = &gShaderEditorInterface;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( ShaderEditorInterface, IVShaderEditor, SHADEREDIT_INTERFACE_VERSION, gShaderEditorInterface );

bool ShaderEditorInterface::bCallbackLock = false;
bool ShaderEditorInterface::bVrCallbackLock = false;

ShaderEditorInterface::ShaderEditorInterface()
	: m_iLastInvalidVarToken( SHADEREDIT_MVAR_TOKEN_INVALID )
	, m_iNextValidVarToken( SHADEREDIT_MVAR_TOKEN_INVALID + 1 )
{
	m_bShowPrimaryDebug = false;

	for ( int i = 0; i < 3; i++ )
		pSkyMats[ i ] = NULL;
}

ShaderEditorInterface::~ShaderEditorInterface()
{
	m_hFunctionCallbacks.PurgeAndDeleteElements();

	//for ( int i = 0; i < 3; i++ )
	//{
	//	if ( pSkyMats[ i ] == NULL )
	//		continue;

	//	pSkyMats[ i ]->Clear();
	//	pSkyMats[ i ] = NULL;
	//}
}

using namespace vgui;

#include "vgui_controls/controls.h"
#include "vgui_controls/panel.h"
#include "vgui_controls/animationcontroller.h"
#include <vgui/isurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include "vgui/ipanel.h"

CViewSetup_SEdit_Shared _MainView;


void RenderFullScreenQuad( IMaterial *pMat )
{
	CMatRenderContextPtr pRenderContext( materials );
	CMeshBuilder pMeshBuilder;
	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, 0, 0, pMat );
	pMeshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	pMeshBuilder.Position3f( -1, 1, 1 );
	pMeshBuilder.TexCoord2f( 0, 0, 0 );
	pMeshBuilder.AdvanceVertex();
	pMeshBuilder.Position3f( 1, 1, 1 );
	pMeshBuilder.TexCoord2f( 0, 1, 0 );
	pMeshBuilder.AdvanceVertex();
	pMeshBuilder.Position3f( 1, -1, 1 );
	pMeshBuilder.TexCoord2f( 0, 1, 1 );
	pMeshBuilder.AdvanceVertex();
	pMeshBuilder.Position3f( -1, -1, 1 );
	pMeshBuilder.TexCoord2f( 0, 0, 1 );
	pMeshBuilder.AdvanceVertex();

	pMeshBuilder.End();
	pMesh->Draw();
}
void GeneralFramebufferUpdate( IMatRenderContext *pRenderContext )
{
	Rect_t srcRect;
	srcRect.x = 0;
	srcRect.y = 0;

	materials->GetBackBufferDimensions( srcRect.width, srcRect.height );

	ITexture *pTexture = GetFBTex();
	int nSrcWidth, nSrcHeight;

	pRenderContext->GetRenderTargetDimensions( nSrcWidth, nSrcHeight );
	pRenderContext->CopyRenderTargetToTextureEx( pTexture, 0, &srcRect );
	pRenderContext->SetFrameBufferCopyTexture( pTexture );
}

void ShaderEditorInterface::OnFrame( float frametime )
{
	if ( IsInEditMode() )
	{
		static ConVarRef qMode( "mat_queue_mode" );
#if defined( SHADER_EDITOR_DLL_SWARM ) || defined( SHADER_EDITOR_DLL_2013 )
		qMode.SetValue( "0" );
#else
		qMode.SetValue( "-1" );
#endif
	}

	IGameSystem::UpdateAllSystems( frametime );
}
void ShaderEditorInterface::OnPreRender( void *viewsetup )
{
	_MainView = *((CViewSetup_SEdit_Shared*)viewsetup);

	IGameSystem::PreRenderAllSystems();
}
void ShaderEditorInterface::OnUpdateSkymask( bool bCombineMode )
{
	SetFramebufferCopyTexOverride( NULL );

	if ( !IsSkymaskAvailable() )
		return;

	//UpdateFramebufferTexture( true );

	ITexture *pSkyMask = GetSkymaskTex();

	if ( IsErrorTexture( pSkyMask ) )
		return;

	static CMaterialReference pMatSkyDraw = NULL;
	static CMaterialReference pMatScreenRestore = NULL;
	static CMaterialReference pMatCombineMasks = NULL;
#ifdef SHADER_EDITOR_DLL_2006
	pMatSkyDraw = materials->FindMaterial( "postprocessing/skymask_fill_0", TEXTURE_GROUP_OTHER );
	pMatCombineMasks = materials->FindMaterial( "postprocessing/skymask_fill_1", TEXTURE_GROUP_OTHER );
	pMatScreenRestore = materials->FindMaterial( "postprocessing/fb_restore", TEXTURE_GROUP_OTHER );
#else
	if ( !pMatSkyDraw )
	{
		pSkyMats[ 0 ] = new KeyValues( "FILL_SKYMASK" );
		pSkyMats[ 0 ]->SetInt( "$COMBINEMODE", 0 );
		pMatSkyDraw.Init( "__sedit_skyfill", TEXTURE_GROUP_CLIENT_EFFECTS, pSkyMats[ 0 ] );
		//pMatSkyDraw->Refresh();
		//pKV->Clear();
		Assert( pMatSkyDraw );
	}
	if ( !pMatScreenRestore )
	{
		pSkyMats[ 1 ] = new KeyValues( "FULLSCREENQUAD_WRITEA" );
		pSkyMats[ 1 ]->SetString( "$BASETEXTURE", GetFBTex()->GetName() );
		pSkyMats[ 1 ]->SetInt( "$COMBINEMODE", 0 );
		pMatScreenRestore.Init( "__sedit_fbrestore", TEXTURE_GROUP_CLIENT_EFFECTS, pSkyMats[ 1 ] );
		//pMatScreenRestore->Refresh();
		//pKV->Clear();
		Assert( pMatScreenRestore );
	}
	if ( !pMatCombineMasks )
	{
		const char *skymaskname = pSkyMask->GetName();
		pSkyMats[ 2 ] = new KeyValues( "FILL_SKYMASK" );
		pSkyMats[ 2 ]->SetString( "$BASETEXTURE", skymaskname );
		pSkyMats[ 2 ]->SetInt( "$COMBINEMODE", 1 );
		pMatCombineMasks.Init( "__sedit_skyfill", TEXTURE_GROUP_CLIENT_EFFECTS, pSkyMats[ 2 ] );
		//pMatCombineMasks->Refresh();
		//pKV->Clear();
		Assert( pMatCombineMasks );
	}
#endif

	// _rt_SEdit_Skymask
	CMatRenderContextPtr pRenderContext( materials );

	IMaterial *pOperation = pMatSkyDraw;
	if ( bCombineMode )
		pOperation = pMatCombineMasks;

	int dest_width = _MainView.width;
	int dest_height = _MainView.height;
	float src_x1 = _MainView.width - 1;
	float src_y1 = _MainView.height - 1;

#ifdef SHADER_EDITOR_DLL_2006
	Frustum frustum;
	CViewSetup setup;
	setup.angles = _MainView.angles;
	setup.origin = _MainView.origin;
	setup.x = _MainView.x;
	setup.y = _MainView.y;
	setup.width = _MainView.width;
	setup.height = _MainView.height;
	setup.fov = _MainView.fov;
	setup.context = 0;
	setup.m_bOrtho = false;
	setup.m_flAspectRatio = _MainView.m_flAspectRatio;
	setup.m_vUnreflectedOrigin = setup.origin;
	render->Push3DView( setup, 0, false, NULL, frustum );
#endif

	pRenderContext->PushRenderTargetAndViewport( NULL );
	//pRenderContext->DepthRange( 0, 1 );
	//MaterialHeightClipMode_t hClipLast = pRenderContext->GetHeightClipMode();
	//pRenderContext->SetHeightClipMode( MATERIAL_HEIGHTCLIPMODE_DISABLE );
	//const bool bEnableClipping = pRenderContext->EnableClipping( false );

#ifndef SHADER_EDITOR_DLL_2006
	pRenderContext->CopyRenderTargetToTexture( GetFBTex() );
#endif

	// do ops
	pRenderContext->DrawScreenSpaceRectangle( pOperation,
		_MainView.x, _MainView.y, dest_width, dest_height,
		0, 0, src_x1, src_y1,
		_MainView.width, _MainView.height );

	// store to mask
	pRenderContext->CopyRenderTargetToTexture( pSkyMask );

	// restore fb
#ifndef SHADER_EDITOR_DLL_2006
	pRenderContext->DrawScreenSpaceRectangle( pMatScreenRestore,
		_MainView.x, _MainView.y, _MainView.width, _MainView.height,
		0, 0, _MainView.width - 1, _MainView.height - 1,
		_MainView.width, _MainView.height );
#endif

	//pRenderContext->EnableClipping( bEnableClipping );
	//pRenderContext->SetHeightClipMode( hClipLast );
	pRenderContext->PopRenderTargetAndViewport();

#ifdef SHADER_EDITOR_DLL_2006
	render->PopView( frustum );
#endif
}
void ShaderEditorInterface::OnSceneRender()
{
	GetPPCache()->RenderAllEffects( true );

	if ( IsInEditMode() )
		::pEditorRoot->OnSceneRender();
}
void ShaderEditorInterface::OnPostRender( bool bUpdateFB )
{
	if ( bUpdateFB )
		UpdateFramebufferTexture( true, false );

	SetFramebufferCopyTexOverride( NULL );

	GetPPCache()->RenderAllEffects( false );

	if ( IsInEditMode() )
	{
		UpdateFramebufferTexture( true );

		::pEditorRoot->OnPostRender();
	}
}

void ShaderEditorInterface::SetFramebufferCopyTexOverride( ITexture *tex )
{
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->SetFrameBufferCopyTexture( (tex!=NULL) ? tex : GetFBTex() );
}
void ShaderEditorInterface::UpdateFramebufferTexture( bool bDoPush, bool bCopyToEditorTarget )
{
	CMatRenderContextPtr pRenderContext( materials );

	if ( bDoPush )
		pRenderContext->PushRenderTargetAndViewport();

	pRenderContext->CopyRenderTargetToTexture( bCopyToEditorTarget ? ::GetFBCopyTex() : GetFBTex() );

	if ( bDoPush )
		pRenderContext->PopRenderTargetAndViewport();
}
void ShaderEditorInterface::RegisterClientCallback( const char *name, pFnClCallback(callback), int numComponents )
{
	char tmp[MAX_PATH];
	::CleanupString( name, tmp, sizeof(tmp) );

	if ( bCallbackLock )
		return Warning( "callbacks have been locked!\n" );
	if ( !callback )
		return Warning( "can't register a null pointer as callback..\n" );
	if ( !name || !Q_strlen( tmp ) )
		return Warning( "can't register callback without name..\n" );

	for ( int i = 0; i < m_hFunctionCallbacks.Count(); i++ )
	{
		if ( !Q_stricmp( m_hFunctionCallbacks[i]->name, tmp ) )
			return Warning( "ambiguous callback name: %s\n", tmp );
	}


	_clCallback *cb = new _clCallback();
	cb->func = callback;
	cb->name = new char[ Q_strlen( tmp ) + 1 ];
	cb->numComps = clamp( numComponents, 1, 4 );
	Q_strcpy( cb->name, tmp );

	m_hFunctionCallbacks.AddToTail( cb );
}
void ShaderEditorInterface::LockClientCallbacks()
{
	bCallbackLock = true;

	Assert(gProcShaderCTRL);

	if ( gProcShaderCTRL )
		gProcShaderCTRL->LinkCallbacks( &m_hFunctionCallbacks );
}
int ShaderEditorInterface::GetNumCallbacks()
{
	return m_hFunctionCallbacks.Count();
}
_clCallback *ShaderEditorInterface::GetCallback( int idx )
{
	Assert( m_hFunctionCallbacks.IsValidIndex( idx ) );
	return m_hFunctionCallbacks[ idx ];
}
int ShaderEditorInterface::FindCallback( const char *name )
{
	if ( !name || !Q_strlen( name ) )
		return -1;
	for ( int i = 0; i < m_hFunctionCallbacks.Count(); i++ )
	{
		if ( !Q_stricmp( m_hFunctionCallbacks[ i ]->name, name ) )
			return i;
	}
	return -1;
}


void ShaderEditorInterface::RegisterViewRenderCallback( const char *pszVrCName, pFnVrCallback(callback),
		const char **pszBoolNames, const bool *pBoolDefaults, const int numBoolParams,
		const char **pszIntNames, const int *pIntDefaults, const int numIntParams,
		const char **pszFloatNames, const float *pFloatDefaults, const int numFloatParams,
		const char **pszStringNames, const char **pStringDefaults, const int numStringParams )
{
	char tmp[MAX_PATH];
	::CleanupString( pszVrCName, tmp, sizeof(tmp) );

	if ( bVrCallbackLock )
		return Warning( "vrcallbacks have been locked!\n" );
	//if ( !callback )
	//	return Warning( "can't register a null pointer as callback..\n" );
	if ( !pszVrCName || !Q_strlen( tmp ) )
		return Warning( "can't register vrcallback without name..\n" );

	for ( int i = 0; i < numBoolParams; i++ )
		if ( !pszBoolNames[i] || Q_strlen( pszBoolNames[i] ) < 1 )
			return Warning( "Bad bool parameter for vrcallback %s at pos %i!\n", pszVrCName, i );

	for ( int i = 0; i < numIntParams; i++ )
		if ( !pszIntNames[i] || Q_strlen( pszIntNames[i] ) < 1 )
			return Warning( "Bad int parameter for vrcallback %s at pos %i!\n", pszVrCName, i );

	for ( int i = 0; i < numFloatParams; i++ )
		if ( !pszFloatNames[i] || Q_strlen( pszFloatNames[i] ) < 1 )
			return Warning( "Bad float parameter for vrcallback %s at pos %i!\n", pszVrCName, i );

	for ( int i = 0; i < numStringParams; i++ )
		if ( !pszStringNames[i] || Q_strlen( pszStringNames[i] ) < 1 )
			return Warning( "Bad string parameter for vrcallback %s at pos %i!\n", pszVrCName, i );


	EditorRenderViewCommand_Definition *pVRCDef = new EditorRenderViewCommand_Definition();

	pVRCDef->SetName( tmp );
	pVRCDef->functor = callback;

	for ( int i = 0; i < numBoolParams; i++ )
	{
		pVRCDef->AddVarToList( EditorRenderViewCommand_Definition::VAR_BOOL, pszBoolNames[i] );

		if ( pBoolDefaults != NULL )
			pVRCDef->defaults.AddBoolValue( pBoolDefaults[i] );
	}

	for ( int i = 0; i < numIntParams; i++ )
	{
		pVRCDef->AddVarToList( EditorRenderViewCommand_Definition::VAR_INT, pszIntNames[i] );

		if ( pIntDefaults != NULL )
			pVRCDef->defaults.AddIntValue( pIntDefaults[i] );
	}

	for ( int i = 0; i < numFloatParams; i++ )
	{
		pVRCDef->AddVarToList( EditorRenderViewCommand_Definition::VAR_FLOAT, pszFloatNames[i] );

		if ( pFloatDefaults != NULL )
			pVRCDef->defaults.AddFloatValue( pFloatDefaults[i] );
	}

	for ( int i = 0; i < numStringParams; i++ )
	{
		pVRCDef->AddVarToList( EditorRenderViewCommand_Definition::VAR_STRING, pszStringNames[i] );

		if ( pStringDefaults != NULL )
			pVRCDef->defaults.AddStringValue( pStringDefaults[i] );
	}

	if ( !GetPPCache()->AddVrCallback( pVRCDef ) )
		return Warning( "duplicate vrcallback name: %s\n", tmp );
}

void ShaderEditorInterface::LockViewRenderCallbacks()
{
	bVrCallbackLock = true;

	GetPPCache()->LockVrCallbacks();
}

int ShaderEditorInterface::GetPPEIndex( const char *pszName )
{
	return GetPPCache()->FindPostProcessingEffect( pszName );
}

bool ShaderEditorInterface::IsPPEEnabled( const int &index )
{
	if ( index < 0 || index >= GetPPCache()->GetNumPostProcessingEffects() )
		return false;

	return GetPPCache()->GetPostProcessingEffect( index )->bIsEnabled;
}

void ShaderEditorInterface::SetPPEEnabled( const int &index, const bool &bEnabled )
{
	if ( index < 0 || index >= GetPPCache()->GetNumPostProcessingEffects() )
		return;

	GetPPCache()->GetPostProcessingEffect( index )->bIsEnabled = bEnabled;
}

IMaterial *ShaderEditorInterface::GetPPEMaterial( const int &index, const char *pszNodeName )
{
	if ( index < 0 || index >= GetPPCache()->GetNumPostProcessingEffects() )
		return NULL;

	return GetPPCache()->GetPostProcessingEffect( index )->FindMaterial( pszNodeName );
}

void ShaderEditorInterface::DrawPPEOnDemand( const int &index, const bool bInScene )
{
	if ( index < 0 || index >= GetPPCache()->GetNumPostProcessingEffects() )
		return;

	GetPPCache()->RenderSinglePPE( GetPPCache()->GetPostProcessingEffect( index ), false, bInScene );
}

IMaterialVar *ShaderEditorInterface::GetPPEMaterialVarFast( ShaderEditVarToken &token,
	const char *pszPPEName, const char *pszNodeName, const char *pszVarName )
{
	if ( token == SHADEREDIT_MVAR_TOKEN_FAILED )
		return NULL;

	if ( token <= m_iLastInvalidVarToken )
	{
		int iEffectIndex = GetPPCache()->FindPostProcessingEffect( pszPPEName );

		if ( iEffectIndex < 0 )
		{
			token = SHADEREDIT_MVAR_TOKEN_FAILED;
			return NULL;
		}

		IMaterial *pMaterial = GetPPCache()->GetPostProcessingEffect( iEffectIndex )->FindMaterial( pszNodeName );

		if ( pMaterial == NULL )
		{
			token = SHADEREDIT_MVAR_TOKEN_FAILED;
			return NULL;
		}

		bool bFound = false;
		IMaterialVar *pVar = pMaterial->FindVar( pszVarName, &bFound );

		if ( pVar == NULL
			|| !bFound )
		{
			token = SHADEREDIT_MVAR_TOKEN_FAILED;
			return NULL;
		}

		token = m_iNextValidVarToken++;
		m_cachedMVars.AddToTail( pVar );
		return pVar;
	}

	int iLocalIndex = token - m_iLastInvalidVarToken - 1;

	Assert( iLocalIndex >= 0 && iLocalIndex < m_cachedMVars.Count() );
	return m_cachedMVars[ iLocalIndex ];
}

void ShaderEditorInterface::FlushPPEMaterialVarCache()
{
	m_iLastInvalidVarToken += m_cachedMVars.Count();

	Assert( m_iNextValidVarToken == m_iLastInvalidVarToken + 1 );

	m_cachedMVars.RemoveAll();
}

MainShaderEditorUpdate __g_ShaderEditorSystem( "shadereditorupd8r" );
MainShaderEditorUpdate *gShaderEditorSystem = &__g_ShaderEditorSystem;

#ifdef SHADER_EDITOR_DLL_2006
void SchemeReloader_Restore( int )
{
	if ( pEditorRoot )
		pEditorRoot->InvalidateLayout( false, true );
}
#endif
void SchemeReloader()
{
	if ( pEditorRoot )
		pEditorRoot->InvalidateLayout( false, true );
}

MainShaderEditorUpdate::MainShaderEditorUpdate( char const *name ) : CAutoGameSystemPerFrame( name )
{
	m_buf_Log.SetBufferType( true, true );

	bDirty = false;
	bCompilerRunning = false;

	_MainView.origin.Init();
	_MainView.angles.Init();
	_MainView.fov = 90;
	_MainView.m_flAspectRatio = 1;
	_MainView.x = _MainView.y = 0;
	_MainView.width = _MainView.height = 256;
	_MainView.zNear = 5;
	_MainView.zFar = 100;
}

MainShaderEditorUpdate::~MainShaderEditorUpdate()
{
	m_buf_Log.Purge();
	ForceTerminateCompilers();
}

bool MainShaderEditorUpdate::Init()
{
	if ( materials && IsInEditMode() )
#ifdef SHADER_EDITOR_DLL_2006
		materials->AddReleaseFunc( &SchemeReloader );
		materials->AddRestoreFunc( &SchemeReloader_Restore );
#else
		materials->AddModeChangeCallBack( SchemeReloader );
#endif
	return true;
}

void MainShaderEditorUpdate::UpdateConstants( const CViewSetup *_override )
{
	CViewSetup_SEdit_Shared setup = CViewSetup_SEdit_Shared( *_override );
	UpdateConstants( &setup );
}
void MainShaderEditorUpdate::UpdateConstants( const CViewSetup_SEdit_Shared *_override )
{
	const CViewSetup_SEdit_Shared *_activeSetup = &_MainView;
	if ( _override )
		_activeSetup = _override;

	Vector _fwd, _right, _up;
	AngleVectors( _activeSetup->angles, &_fwd, &_right, &_up );

	for ( int i = HLSLENV_AUTOCOPYCONST_FIRST; i <= HLSLENV_AUTOCOPYCONST_LAST; i++ )
	{
		float _data[4] = { 0,0,0,0 };

		switch ( i )
		{
		default:
			Assert(0);
			break;
		case HLSLENV_VIEW_ORIGIN:
			AutoCopyFloats( _activeSetup->origin.Base(), _data, 3 );
			break;
		case HLSLENV_VIEW_FWD:
			AutoCopyFloats( _fwd.Base(), _data, 3 );
			break;
		case HLSLENV_VIEW_RIGHT:
			AutoCopyFloats( _right.Base(), _data, 3 );
			break;
		case HLSLENV_VIEW_UP:
			AutoCopyFloats( _up.Base(), _data, 3 );
			break;
		case HLSLENV_VIEW_WORLDDEPTH:
			_data[0] = _activeSetup->zFar;
			break;
		}

		gProcShaderCTRL->UpdateEnvironmentData( i, _data );
	}
}
void MainShaderEditorUpdate::PreRender()
{
	if ( !gProcShaderCTRL )
		return;
	UpdateConstants();
}

void DoMaterialReload()
{
	g_pMaterialSystem->UncacheAllMaterials();
	g_pMaterialSystem->CacheUsedMaterials();
	g_pMaterialSystem->ReloadMaterials();
}

static vgui::PromptSimple *prompt = NULL;
static int iMatReloadStep = 0;

void MainShaderEditorUpdate::BeginMaterialReload( bool bResetMode )
{
	if ( pEditorRoot )
	{
		if ( prompt )
			prompt->MarkForDeletion();

		prompt = new vgui::PromptSimple( pEditorRoot, "" );
		prompt->SetText( "Reloading materials..." );

		prompt->MakeReadyForUse();
		prompt->InvalidateLayout( true, true );
		prompt->QuickCenterLayout();

		prompt->DisableFadeEffect();
		prompt->SetAlpha( 255 );
		prompt->SetBgColor( Color( 96, 64, 64, 200 ) );
		prompt->MoveToCenterOfScreen();

		iMatReloadStep = bResetMode ? 2 : 1;
	}
}
void MainShaderEditorUpdate::Update( float frametime )
{
#if 0
	ITexture *pBlah = materials->FindTexture( "_rt_vst", TEXTURE_GROUP_RENDER_TARGET );
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->PushRenderTargetAndViewport( pBlah );
	pRenderContext->ClearColor4ub( RandomInt(0,255), RandomInt(0,255), RandomInt(0,255), RandomInt(0,255) );
	pRenderContext->ClearBuffers( true, false );
	pRenderContext->PopRenderTargetAndViewport();
#endif

	vgui::GetAnimationController()->UpdateAnimations( GetEditorTime() ); //engine->Time() );
	if ( IsLogDirty() )
	{
		char _out[1024];
		ParseLog( _out, sizeof( _out ) );
		Msg("%s",_out);
	}

	if ( !IsInEditMode() )
		return;

	if ( iMatReloadStep )
	{
		if ( prompt )
		{
			prompt->MarkForDeletion();
			prompt = NULL;
		}
		ReloadGameShaders( (iMatReloadStep==1)?pEditorRoot->GetLastFullyCompiledShader():NULL,
			pEditorRoot->GetReloadMaterialList(), pEditorRoot->GetNumReloadMaterials() );
		DoMaterialReload();
		iMatReloadStep = 0;
		return;
	}

	static bool testKeys[ 3 ] = {
		false,
		false,
		false,
	};

	if ( !!input()->GetAppModalSurface() )
		return;

	bool bLocal = input()->IsKeyDown( KEY_F1 );
	if ( bLocal != testKeys[ 0 ] )
	{
		testKeys[ 0 ] = bLocal;
		if ( testKeys[ 0 ] )
			pEditorRoot->ToggleFullScreen();
	}

	bLocal = input()->IsKeyDown( KEY_F2 );
	if ( bLocal != testKeys[ 1 ] )
	{
		testKeys[ 1 ] = bLocal;
		if ( testKeys[ 1 ] )
			pEditorRoot->ToggleInput();
	}

	bLocal = input()->IsKeyDown( KEY_F3 );
	if ( bLocal != testKeys[ 2 ] )
	{
		testKeys[ 2 ] = bLocal;
		if ( testKeys[ 2 ] )
			pEditorRoot->ToggleVisible();
	}

	if ( input()->IsKeyDown( KEY_LCONTROL ) && 
		input()->IsKeyDown( KEY_LSHIFT ) &&
		input()->IsKeyDown( KEY_LALT ) &&
		input()->IsKeyDown( KEY_V ) )
		pEditorRoot->ActivateBuildMode();
}

void MainShaderEditorUpdate::QueueLog( const char *_data, int len )
{
	m_BufferLock.Lock();

	if ( !m_buf_Log.IsValid() )
		m_buf_Log.Purge();
	m_buf_Log.Put( _data, len );
	bDirty = true;

	m_BufferLock.Unlock();
}
void MainShaderEditorUpdate::QueueLog( const char *_data )
{
	m_BufferLock.Lock();

	if ( !m_buf_Log.IsValid() )
		m_buf_Log.Purge();
	m_buf_Log.PutString( _data );
	bDirty = true;

	m_BufferLock.Unlock();
}
void MainShaderEditorUpdate::ParseLog( char *logOut, int maxLen )
{
	m_BufferLock.Lock();

	int size = min( maxLen-1, m_buf_Log.TellPut() );
	m_buf_Log.Get( logOut, size );
	logOut[ size ] = '\0';
	m_buf_Log.Purge();
	bDirty = false;

	m_BufferLock.Unlock();
}
bool MainShaderEditorUpdate::IsLogDirty()
{
	bool bOut = false;
	m_BufferLock.Lock();
	bOut = bDirty;
	m_BufferLock.Unlock();
	return bOut;
}


void MainShaderEditorUpdate::SetCompilerState( bool bRunning )
{
	m_LockCompilerState.Lock();
	bCompilerRunning = bRunning;
	m_LockCompilerState.Unlock();
}
bool MainShaderEditorUpdate::IsCompilerRunning()
{
	m_LockCompilerState.Lock();
	bool bReturn = bCompilerRunning;
	m_LockCompilerState.Unlock();
	return bReturn;
}

void CheckDirectories()
{
	Assert( pEditorRoot );

	char checkSCompile[MAX_PATH];
	Q_snprintf( checkSCompile, MAX_PATH, "%s\\shadercompile.exe", GetBinaryPath() );
	if ( !g_pFullFileSystem->FileExists( checkSCompile ) )
	{
		vgui::PromptSimple *prompt = new vgui::PromptSimple( pEditorRoot, "ERROR" );

		int screenx,screeny;
		engine->GetScreenSize(screenx,screeny);
		screenx /= 2;
		screeny /= 2;
		int promptsizex = 240;
		int promptsizey = 200;
		screenx -= promptsizex / 2;
		screeny -= promptsizey / 2;

		prompt->SetSize( promptsizex, promptsizey );
		prompt->SetPos( screenx,screeny );

		char errormsg[MAXTARGC];
		if ( !g_pFullFileSystem->IsDirectory( GetBinaryPath() ) )
			Q_snprintf( errormsg, MAXTARGC, "Binaries cannot be located. Please run Source SDK once." );
		else
			Q_snprintf( errormsg, MAXTARGC, "shadercompile.exe cannot be found at %s! Try reinstalling the Source SDK.", GetBinaryPath() );

		prompt->SetText( errormsg );
		prompt->AddButton( "Ok", "onclose_prompt" );

		prompt->MakeReadyForUse();
		prompt->InvalidateLayout( true, true );
	}
}

void ReloadGameShaders( GenericShaderData *data, char **pszMaterialList, int iNumMaterials )
{
	const bool bIsOverriding = !!data;
	char *CanvasName = NULL;

	if ( bIsOverriding )
	{
		const char *ShaderName = data->name;

		int SwapIndex = ::gProcShaderCTRL->FindPreloadShader( ShaderName );
		if ( SwapIndex < 0 )
		{
			const char *pszSearch = Q_stristr( data->name, "_" );
			if ( pszSearch )
			{
				const char *ShaderName_Small = pszSearch + 1;
				SwapIndex = ::gProcShaderCTRL->FindPreloadShader( ShaderName_Small );
			}
		}

		CanvasName = new char[ Q_strlen( ShaderName ) + 1 ];
		Q_strcpy( CanvasName, ShaderName );

		BasicShaderCfg_t *SwapData = new BasicShaderCfg_t( *data->shader );
		Assert( !SwapData->CanvasName );
		SwapData->CanvasName = CanvasName;

		// build shader names
		static unsigned int _HackyIndexIncrement = 0;
		{
			int len = Q_strlen( SwapData->ProcPSName ) + 2 + 7;
			char *uniquifyShader = new char[len];
			Q_snprintf( uniquifyShader, len, "%06X_%s", _HackyIndexIncrement, SwapData->ProcPSName );
			delete [] SwapData->ProcPSName;
			SwapData->ProcPSName = uniquifyShader;
		}
		{
			int len = Q_strlen( SwapData->ProcVSName ) + 2 + 7;
			char *uniquifyShader = new char[len];
			Q_snprintf( uniquifyShader, len, "%06X_%s", _HackyIndexIncrement, SwapData->ProcVSName );
			delete [] SwapData->ProcVSName;
			SwapData->ProcVSName = uniquifyShader;
		}

		char *OldName = data->name;
		char targetname[MAX_PATH];
		Q_snprintf( targetname, MAX_PATH, "%06X_%s", _HackyIndexIncrement, OldName );

		// copy vcs files
		char __path_compiler[MAX_PATH];
		char __path_engine[MAX_PATH];
		ComposeShaderPath_Compiled( data, false, true, __path_compiler, MAX_PATH );
		data->name = targetname;
		ComposeShaderPath_CompiledEngine( data, false, true, __path_engine, MAX_PATH );
		EngineCopy( __path_compiler, __path_engine );

		data->name = OldName;
		ComposeShaderPath_Compiled( data, true, true, __path_compiler, MAX_PATH );
		data->name = targetname;
		ComposeShaderPath_CompiledEngine( data, true, true, __path_engine, MAX_PATH );
		EngineCopy( __path_compiler, __path_engine );


		data->name = OldName;
		_HackyIndexIncrement++;

		if ( SwapIndex >= 0 )
		{
			// swap shader data
			BasicShaderCfg_t *trash = (BasicShaderCfg_t*)gProcShaderCTRL->SwapPreloadShader( SwapIndex, SwapData );
			delete trash;
		}
		else
			gProcShaderCTRL->AddPreloadShader( SwapData );
	}

	// reload materials
	if ( pszMaterialList && iNumMaterials )
	{
		for ( int i = 0; i < iNumMaterials; i++ )
		{
			IMaterial *pMat = materials->FindMaterial( pszMaterialList[i], TEXTURE_GROUP_MODEL );
			if ( IsErrorMaterial( pMat ) )
				continue;

			KeyValues *pKV = new KeyValues( "" );
			char tmppath[ MAX_PATH ];
			Q_snprintf( tmppath, MAX_PATH, "materials/%s.vmt", pszMaterialList[i] );
			if ( !pKV->LoadFromFile( g_pFullFileSystem, tmppath ) )
			{
				pKV->deleteThis();
				continue;
			}

			if ( bIsOverriding )
			{
				pKV->SetName( "EDITOR_SHADER" );
				pKV->SetString( "$SHADERNAME", CanvasName );
			
				const char *curEnvmap = pKV->GetString( "$envmap" );
				if ( !curEnvmap || !*curEnvmap )
					pKV->SetString( "$envmap", "env_cubemap" );
			}
			else
				UnpackMaterial( pKV );

			pMat->SetShaderAndParams( pKV );

			pMat->Refresh();
			pMat->RecomputeStateSnapshots();

			pKV->deleteThis();
		}
		return;
	}

	if ( !bIsOverriding )
		return;

	QuickRefreshEditorShader();
}
void QuickRefreshEditorShader()
{
	const char *nameLast = NULL;
	for ( MaterialHandle_t curMatHandle = g_pMaterialSystem->FirstMaterial();
		curMatHandle != g_pMaterialSystem->InvalidMaterial();
		curMatHandle = g_pMaterialSystem->NextMaterial( curMatHandle ) )
	{
		IMaterial *pMat = g_pMaterialSystem->GetMaterial( curMatHandle );
		if ( IsErrorMaterial( pMat ) )
			continue;

		//bool bSanityTest = false;
		//IMaterialVar *pVarBaseTexture = pMat->FindVar( "$basetexture", &bSanityTest, false );
		//if ( !pVarBaseTexture || !bSanityTest )
		//	continue;
		if ( !pMat->ShaderParamCount() )
			continue;

		const char *n = pMat->GetName();

		if ( !n || (nameLast && !Q_strcmp( nameLast, n )) )
			continue;
		nameLast = n;

		const char *shaderName = pMat->GetShaderName();
		if ( !shaderName )
			continue;

		if ( !Q_stricmp( shaderName, "EDITOR_SHADER" ) )
		{
			pMat->Refresh();
			pMat->RecomputeStateSnapshots();
		}
	}
}