
#include "cbase.h"
#include "editorcommon.h"
#include "dll_init.h"

#include "steam/steam_api.h"
#include "datacache/idatacache.h"
#include "engine/ienginesound.h"
#include "igameuifuncs.h"
#include "inputsystem/iinputsystem.h"
#include "client_factorylist.h"
//#include "initializer.h"
#include "vgui_init.h"
#include "tier0/icommandline.h"

#ifdef SHADER_EDITOR_DLL_SWARM
#include "interfaces/interfaces.h"
#endif

#ifdef SHADER_EDITOR_DLL_2006
#include "cdll_convar.h"

IMDLCache *mdlcache = NULL;
static CDLL_ConVarAccessor g_ConVarAccessor;
#endif

IVEngineClient	*engine = NULL;
IVModelRender *modelrender = NULL;
IVRenderView *render = NULL;
IDataCache *datacache = NULL;
IVModelInfoClient *modelinfo = NULL;
IEngineVGui *enginevgui = NULL;
ISpatialPartition* partition = NULL;
IFileSystem *filesystem = NULL;
IEngineSound *enginesound = NULL;
IUniformRandomStream *random = NULL;
static CGaussianRandomStream s_GaussianRandomStream;
CGaussianRandomStream *randomgaussian = &s_GaussianRandomStream;
IGameUIFuncs *gameuifuncs = NULL;
IInputSystem *inputsystem = NULL;
IUploadGameStats *gamestatsuploader = NULL;

#ifndef SHADER_EDITOR_DLL_2006
ISoundEmitterSystemBase *soundemitterbase = NULL;
static CSteamAPIContext g_SteamAPIContext;
CSteamAPIContext *steamapicontext = &g_SteamAPIContext;
#endif

static CGlobalVarsBase dummyvars( true );
CGlobalVarsBase *gpGlobals = &dummyvars;




#include "EditorInit.h"
#include "editorcommon.h"
#include "IVProcShader.h"

CSysModule *ProcShaderModule = NULL;
IVProcShader *gProcShaderCTRL = NULL;
ISEditModelRender *sEditMRender = NULL;

static bool bIsEditing = false;
const bool IsInEditMode()
{
	return bIsEditing;
}

static void OnShaderEditor_RestoreFunc( int nChangeFlags )
{
	shaderEdit->FlushPPEMaterialVarCache();
}


bool ShaderEditorInterface::Init( CreateInterfaceFn appSystemFactory, CGlobalVarsBase *pGlobals, void *pSEditMRender,
								bool bCreateEditor, bool bEnablePrimaryDebug, int iSkymaskMode )
{
#if DEBUG
	//ToggleMemDbgLock();
#endif

	sEditMRender = (ISEditModelRender*)pSEditMRender;

	bIsEditing = bCreateEditor;
	m_bShowPrimaryDebug = bEnablePrimaryDebug;

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );

	gpGlobals = pGlobals;

#ifdef SHADER_EDITOR_DLL_SWARM
	ConnectInterfaces( &appSystemFactory, 1 );
#endif

	ConnectTier1Libraries( &appSystemFactory, 1 );
	ConnectTier2Libraries( &appSystemFactory, 1 );
	ConnectTier3Libraries( &appSystemFactory, 1 );

	if ( (engine = (IVEngineClient *)appSystemFactory( VENGINE_CLIENT_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( (modelrender = (IVModelRender *)appSystemFactory( VENGINE_HUDMODEL_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( (render = (IVRenderView *)appSystemFactory( VENGINE_RENDERVIEW_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( (datacache = (IDataCache*)appSystemFactory(DATACACHE_INTERFACE_VERSION, NULL )) == NULL )
		return false;
#ifdef SHADER_EDITOR_DLL_2006
	if ( (mdlcache = (IMDLCache*)appSystemFactory( MDLCACHE_INTERFACE_VERSION, NULL )) == NULL )
		return false;
#else
	if ( !mdlcache )
		return false;
#endif
	if ( (modelinfo = (IVModelInfoClient *)appSystemFactory(VMODELINFO_CLIENT_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( (enginevgui = (IEngineVGui *)appSystemFactory(VENGINE_VGUI_VERSION, NULL )) == NULL )
		return false;
	if ( (enginesound = (IEngineSound *)appSystemFactory(IENGINESOUND_CLIENT_INTERFACE_VERSION, NULL)) == NULL )
		return false;
#ifdef SHADER_EDITOR_DLL_2006
	setdllFS( (IFileSystem *)appSystemFactory(FILESYSTEM_INTERFACE_VERSION, NULL) );
	if ( g_pFullFileSystem == NULL )
		return false;
#else
	if ( (filesystem = (IFileSystem *)appSystemFactory(FILESYSTEM_INTERFACE_VERSION, NULL)) == NULL )
		return false;
#endif
	if ( (random = (IUniformRandomStream *)appSystemFactory(VENGINE_CLIENT_RANDOM_INTERFACE_VERSION, NULL)) == NULL )
		return false;
	if ( (gameuifuncs = (IGameUIFuncs * )appSystemFactory( VENGINE_GAMEUIFUNCS_VERSION, NULL )) == NULL )
		return false;
	if ( (inputsystem = (IInputSystem *)appSystemFactory(INPUTSYSTEM_INTERFACE_VERSION, NULL)) == NULL )
		return false;

	factorylist_t factories;
	factories.appSystemFactory = appSystemFactory;
	FactoryList_Store( factories );

	if( !g_pMaterialSystemHardwareConfig )
		return false;

	s_GaussianRandomStream.AttachToStream( random );

#ifdef SHADER_EDITOR_DLL_2006
	//if ( bCreateEditor )
	//	ConCommandBaseMgr::OneTimeInit(&g_ConVarAccessor);
#else
	ConVar_Register( FCVAR_CLIENTDLL );
#endif

	CreateDirectoryStrings();

	//if (!Initializer::InitializeAllObjects())
	//	return false;

	if (!VGui_Editor_Startup( appSystemFactory ))
		return false;

#ifdef SHADER_EDITOR_DLL_SWARM
	const char *pszModuleName = "game_shader_generic_eshader_SWARM.dll";
#elif SHADER_EDITOR_DLL_2006
	const char *pszModuleName = "game_shader_generic_eshader_2006.dll";
#elif SHADER_EDITOR_DLL_2013
	//const char *pszModuleName = "game_shader_generic_eshader_2013.dll";
	const char *pszModuleName = "game_shader_dx6.dll";
#elif SHADER_EDITOR_DLL_PORTAL2
	const char *pszModuleName = "game_shader_generic_eshader_PORTAL2.dll";
#else
	const char *pszModuleName = "game_shader_generic_eshader_2007.dll";
#endif

	char modulePath[MAX_PATH];
	Q_snprintf( modulePath, sizeof( modulePath ), "%s/bin/%s", engine->GetGameDirectory(), pszModuleName );
	ProcShaderModule = Sys_LoadModule( modulePath );
	if ( ProcShaderModule )
	{
		CreateInterfaceFn ProcShaderFactory = Sys_GetFactory( ProcShaderModule );
		gProcShaderCTRL = ProcShaderFactory ? ((IVProcShader *) ProcShaderFactory( PROCSHADER_INTERFACE_VERSION, NULL )) : NULL;
	}

	if ( !gProcShaderCTRL )
	{
		Warning( "Cannot load %s.\n", pszModuleName );
		return false;
	}
	else if ( !gProcShaderCTRL->Init( appSystemFactory, gPPEHelper ) )
	{
		Warning( "Cannot init shader interface.\n" );
		return false;
	}

	if ( !IGameSystem::InitAllSystems() )
		return false;

	ICommandLine *pCom = CommandLine();
	const int iRtDefaultSize = 1024;
	const int iRtSize = pCom ? pCom->ParmValue( "-graphrtsize", iRtDefaultSize ) : iRtDefaultSize;

	if ( bCreateEditor )
		AllocRTs( iRtSize );

	AllocSkymask( (SEDIT_SKYMASK_MODE)iSkymaskMode );

	PreviewCleanup();
#if 0 //def SHADER_EDITOR_DLL_SWARM
	InitSwarmShaders();
#endif

	VGui_Editor_CreateGlobalPanels( bCreateEditor );

	if ( bCreateEditor )
		ResetTileInfo();

	if ( bCreateEditor )
		CheckDirectories();

	materials->AddRestoreFunc( &OnShaderEditor_RestoreFunc );

	return true;
}

void ShaderEditorInterface::PrecacheData()
{
	static bool bNoDoublePrecache = false;
	if ( bNoDoublePrecache )
	{
		Warning( "Don't call PrecacheData() twice.\n" );
		return;
	}

	bNoDoublePrecache = true;

	LoadGameShaders( true );
	GetPPCache()->LoadPrecacheFile();
}

void ShaderEditorInterface::OverridePaths( const char *pszWorkingDirectory, const char *pszCompilePath, const char *pszLocalCompilePath,
		const char *pszGamePath, const char *pszCanvas, const char *pszShaderSource, const char *pszDumps,
		const char *pszUserFunctions, const char *pszEditorRoot )
{
	OverrideDirectories( pszWorkingDirectory, pszCompilePath, pszLocalCompilePath, pszGamePath,
		pszCanvas, pszShaderSource, pszDumps, pszUserFunctions, pszEditorRoot );
}

void ShaderEditorInterface::Shutdown()
{
	materials->RemoveRestoreFunc( &OnShaderEditor_RestoreFunc );

	GetMatSysShutdownHelper()->Shutdown();

	UnloadGameShaders();

	if ( gProcShaderCTRL )
		gProcShaderCTRL->Shutdown();

	PreviewCleanup();

	//Initializer::FreeAllObjects();

	IGameSystem::ShutdownAllSystems();

	VGui_Editor_Shutdown();

	DisconnectTier3Libraries( );
	DisconnectTier2Libraries( );
#ifndef SHADER_EDITOR_DLL_2006
	ConVar_Unregister();
#endif
	DisconnectTier1Libraries( );

#ifdef SHADER_EDITOR_DLL_SWARM
	DisconnectInterfaces();
#endif

#if DEBUG
	//ToggleMemDbgLock();
#endif
}

char *VarArgs( char *format, ... )
{
	va_list		argptr;
	static char		string[1024];
	
	va_start (argptr, format);
	Q_vsnprintf (string, sizeof( string ), format,argptr);
	va_end (argptr);

	return string;	
}

int ScreenHeight( void )
{
	int w, h;
	engine->GetScreenSize( w, h );
	return h;
}

int ScreenWidth( void )
{
	int w, h;
	engine->GetScreenSize( w, h );
	return w;
}