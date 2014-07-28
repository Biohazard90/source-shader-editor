
#include "cbase.h"

#include <direct.h>

#include "editorCommon.h"

using namespace std;

#define MASTERPATH_MAXLEN 512

static char _wd[MASTERPATH_MAXLEN];
static char _acc[MASTERPATH_MAXLEN];
static char _compilePath[MASTERPATH_MAXLEN];
static char _compilePath_Local[MASTERPATH_MAXLEN];
static char _gamePath[MASTERPATH_MAXLEN];
static char _canvasDir[MASTERPATH_MAXLEN];
static char _shaderSource[MASTERPATH_MAXLEN];
static char _DumpFiles[MASTERPATH_MAXLEN];
static char _uFuncs[MASTERPATH_MAXLEN];
static char _seditRoot[MASTERPATH_MAXLEN];

#ifdef SHADER_EDITOR_DLL_SWARM
static char _swarmShaderDir[MASTERPATH_MAXLEN];
#endif

static char _wd_override[MASTERPATH_MAXLEN] = { 0 }; // better safe than sorry
static char _compilePath_override[MASTERPATH_MAXLEN] = { 0 };
static char _compilePath_Local_override[MASTERPATH_MAXLEN] = { 0 };
static char _gamePath_override[MASTERPATH_MAXLEN] = { 0 };
static char _canvasDir_override[MASTERPATH_MAXLEN] = { 0 };
static char _shaderSource_override[MASTERPATH_MAXLEN] = { 0 };
static char _DumpFiles_override[MASTERPATH_MAXLEN] = { 0 };
static char _uFuncs_override[MASTERPATH_MAXLEN] = { 0 };
static char _seditRoot_override[MASTERPATH_MAXLEN] = { 0 };

void OverrideDirectories( const char *pszWorkingDirectory, const char *pszCompilePath, const char *pszLocalCompilePath,
		const char *pszGamePath, const char *pszCanvas, const char *pszShaderSource, const char *pszDumps,
		const char *pszUserFunctions, const char *pszEditorRoot )
{
	if ( pszWorkingDirectory != NULL )
		Q_strncpy( _wd_override, pszWorkingDirectory, sizeof( _wd_override ) );

	if ( pszCompilePath != NULL )
		Q_strncpy( _compilePath_override, pszCompilePath, sizeof( _compilePath_override ) );

	if ( pszLocalCompilePath != NULL )
		Q_strncpy( _compilePath_Local_override, pszLocalCompilePath, sizeof( _compilePath_Local_override ) );

	if ( pszGamePath != NULL )
		Q_strncpy( _gamePath_override, pszGamePath, sizeof( _gamePath_override ) );

	if ( pszCanvas != NULL )
		Q_strncpy( _canvasDir_override, pszCanvas, sizeof( _canvasDir_override ) );

	if ( pszShaderSource != NULL )
		Q_strncpy( _shaderSource_override, pszShaderSource, sizeof( _shaderSource_override ) );

	if ( pszDumps != NULL )
		Q_strncpy( _DumpFiles_override, pszDumps, sizeof( _DumpFiles_override ) );

	if ( pszUserFunctions != NULL )
		Q_strncpy( _uFuncs_override, pszUserFunctions, sizeof( _uFuncs_override ) );

	if ( pszEditorRoot != NULL )
		Q_strncpy( _seditRoot_override, pszEditorRoot, sizeof( _seditRoot_override ) );
}

void ApplyDirectoryOverrides()
{
#define _OVERRIDE_DIR( x ) \
	if ( *(x##_override) ) \
		Q_strncpy( x, x##_override, sizeof( x ) );

	_OVERRIDE_DIR( _wd );
	_OVERRIDE_DIR( _compilePath );
	_OVERRIDE_DIR( _compilePath_Local );
	_OVERRIDE_DIR( _gamePath );
	_OVERRIDE_DIR( _canvasDir );
	_OVERRIDE_DIR( _shaderSource );
	_OVERRIDE_DIR( _DumpFiles );
	_OVERRIDE_DIR( _uFuncs );
	_OVERRIDE_DIR( _seditRoot );
}

void CreateDirectoryStrings()
{
	DestroyDirectoryStrings();

	char _wdTmp[ MASTERPATH_MAXLEN ];
	char szSearchPath[ MASTERPATH_MAXLEN ];

	if ( g_pFullFileSystem->GetSearchPath( "EXECUTABLE_PATH", false, szSearchPath, sizeof( szSearchPath ) ) > 0 )
	{
		Q_strncpy( _wdTmp, szSearchPath, MASTERPATH_MAXLEN );
	}
	else
	{
		*szSearchPath = 0;

		if ( !g_pFullFileSystem->GetCurrentDirectory( _wdTmp, MASTERPATH_MAXLEN  ) )
			getcwd( _wdTmp, sizeof( _wdTmp ) );
	}

	Q_FixSlashes( _wdTmp );

	Q_strncpy( _wd, _wdTmp, MASTERPATH_MAXLEN );

	char *sapps = Q_stristr( _wdTmp, "steamapps" );

	if ( sapps )
	{
		sapps = Q_StripFirstDir( sapps );
		Q_IsolateFirstDir( sapps );
		Q_snprintf( _acc, MASTERPATH_MAXLEN, "%s", sapps );

		*sapps = '\0';
	}
	else
	{
		Warning( "Unable to find account name/common\n" );
		Q_snprintf( _acc, MASTERPATH_MAXLEN, "common" );
	}
	
	//char *smods = Q_stristr( _wdTmp, "SourceMods" );
	const char *_game = engine->GetGameDirectory();

	Q_snprintf( _gamePath, MASTERPATH_MAXLEN, "%s", _game );
	Q_FixSlashes( _gamePath );
	
	KeyValues *pKV = new KeyValues("cfg");
	const char *localize = "..\\..\\..\\..\\";

#ifdef SHADER_EDITOR_DLL_SWARM
	const char *defCMP = "alien swarm/bin";
	const char *defCMPKey = "compiler_swarm";
#elif SHADER_EDITOR_DLL_2006
	const char *defCMP = "sourcesdk/bin/ep1/bin";
	const char *defCMPKey = "compiler_2006";
#elif SHADER_EDITOR_DLL_2013
	const char *defCMP = "";
	const char *defCMPKey = "compiler_2013";
#else
	const char *defCMP = "sourcesdk/bin/source2007/bin";
	const char *defCMPKey = "compiler";
#endif

	const char *_cmp = defCMP;
	if ( pKV->LoadFromFile( g_pFullFileSystem, "shadereditorui/path_config.txt", "MOD" ) )
		_cmp = pKV->GetString( defCMPKey, defCMP );

#ifdef SHADER_EDITOR_DLL_2013
	Q_snprintf( _compilePath, MASTERPATH_MAXLEN, "%s/bin", _wd );
#else
	Q_snprintf( _compilePath, MASTERPATH_MAXLEN, "%s%s/%s", _wdTmp, _acc, _cmp );
#endif

#ifdef SHADER_EDITOR_DLL_2013
	char szCurrentDir[ MASTERPATH_MAXLEN ];
	Q_FileBase( _wd, szCurrentDir, MASTERPATH_MAXLEN );
	Q_snprintf( _compilePath_Local, MASTERPATH_MAXLEN, "%scommon\\%s\\bin", localize, szCurrentDir );
#else
	Q_snprintf( _compilePath_Local, MASTERPATH_MAXLEN, "%s%s\\%s", localize, _acc, _cmp );
#endif

#ifdef SHADER_EDITOR_DLL_2013
	if ( *szSearchPath != 0 )
	{
		Q_strncpy( _compilePath, szSearchPath, sizeof( _compilePath ) );
		Q_strncpy( _compilePath_Local, szSearchPath, sizeof( _compilePath_Local ) );
	}
#endif

	Q_FixSlashes( _compilePath );
	Q_FixSlashes( _compilePath_Local );
	pKV->deleteThis();

	
	const char *__localrootdir = "shadereditorui";
	Q_snprintf( _seditRoot, MASTERPATH_MAXLEN, "%s/%s", _gamePath, __localrootdir );
	Q_FixSlashes( _seditRoot );

	const char *__localcanvasdir = "canvas";
	Q_snprintf( _canvasDir, MASTERPATH_MAXLEN, "%s/%s", _seditRoot, __localcanvasdir );
	Q_FixSlashes( _canvasDir );

	const char *__localsrcdir = "shader_src";
	Q_snprintf( _shaderSource, MASTERPATH_MAXLEN, "%s/%s", _seditRoot, __localsrcdir );
	Q_FixSlashes( _shaderSource );

	const char *__localdumpdir = "dumps";
	Q_snprintf( _DumpFiles, MASTERPATH_MAXLEN, "%s/%s", _seditRoot, __localdumpdir );
	Q_FixSlashes( _DumpFiles );

	const char *__localufuncdir = "user_functions";
	Q_snprintf( _uFuncs, MASTERPATH_MAXLEN, "%s/%s", _seditRoot, __localufuncdir );
	Q_FixSlashes( _uFuncs );

#ifdef SHADER_EDITOR_DLL_SWARM
	const char *__swarmShaderTarget = "platform/shaders/fxc";
	Q_snprintf( _swarmShaderDir, MASTERPATH_MAXLEN, "%s/%s", _wd, __swarmShaderTarget );
	Q_FixSlashes( _swarmShaderDir );
#endif


	ApplyDirectoryOverrides();

	CreateDirectories();


	if ( shaderEdit->ShouldShowPrimaryDbg() )
	{
		Msg( "wd: %s\n", _wd );
#ifdef SHADER_EDITOR_DLL_SWARM
		Msg( "swarm parent name: %s\n", _acc );
		Msg( "swarm shader path: %s\n", _swarmShaderDir );
#else
		Msg( "acc: %s\n", _acc );
#endif
		Msg( "game: %s\n", _gamePath );
		Msg( "editor root: %s\n", _seditRoot );
		Msg( "binaries: %s\n", _compilePath );
		Msg( "binaries local: %s\n", _compilePath_Local );
		Msg( "canvas: %s\n", _canvasDir );
		Msg( "funcs: %s\n", _uFuncs );
		Msg( "src: %s\n", _shaderSource );
	}
}
void CreateDirectories()
{
	if ( !g_pFullFileSystem->IsDirectory( _canvasDir, "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( _canvasDir, "MOD" );

	if ( !g_pFullFileSystem->IsDirectory( _DumpFiles, "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( _DumpFiles, "MOD" );
	if ( !g_pFullFileSystem->IsDirectory( _shaderSource, "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( _shaderSource, "MOD" );

	char tmpDir[MASTERPATH_MAXLEN];
	Q_snprintf( tmpDir, sizeof( tmpDir ), "%s/shaders/fxc", _shaderSource );
	Q_FixSlashes( tmpDir );
	if ( !g_pFullFileSystem->IsDirectory( tmpDir, "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( tmpDir, "MOD" );
	Q_snprintf( tmpDir, sizeof( tmpDir ), "%s/shaders/psh", _shaderSource );
	Q_FixSlashes( tmpDir );
	if ( !g_pFullFileSystem->IsDirectory( tmpDir, "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( tmpDir, "MOD" );
	Q_snprintf( tmpDir, sizeof( tmpDir ), "%s/shaders/vsh", _shaderSource );
	Q_FixSlashes( tmpDir );
	if ( !g_pFullFileSystem->IsDirectory( tmpDir, "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( tmpDir, "MOD" );


	Q_snprintf( tmpDir, sizeof( tmpDir ), "%s/shaders", _gamePath );
	Q_FixSlashes( tmpDir );
	if ( !g_pFullFileSystem->IsDirectory( tmpDir, "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( tmpDir, "MOD" );
	Q_snprintf( tmpDir, sizeof( tmpDir ), "%s/shaders/fxc", _gamePath );
	Q_FixSlashes( tmpDir );
	if ( !g_pFullFileSystem->IsDirectory( tmpDir, "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( tmpDir, "MOD" );
	Q_snprintf( tmpDir, sizeof( tmpDir ), "%s/shaders/psh", _gamePath );
	Q_FixSlashes( tmpDir );
	if ( !g_pFullFileSystem->IsDirectory( tmpDir, "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( tmpDir, "MOD" );
	Q_snprintf( tmpDir, sizeof( tmpDir ), "%s/shaders/vsh", _gamePath );
	Q_FixSlashes( tmpDir );
	if ( !g_pFullFileSystem->IsDirectory( tmpDir, "MOD" ) )
		g_pFullFileSystem->CreateDirHierarchy( tmpDir, "MOD" );

#ifdef SHADER_EDITOR_DLL_SWARM
	if ( !g_pFullFileSystem->IsDirectory( _shaderSource ) )
		g_pFullFileSystem->CreateDirHierarchy( _shaderSource );

	Q_snprintf( tmpDir, sizeof( tmpDir ), "%s/fxc", _shaderSource );
	Q_FixSlashes( tmpDir );
	if ( !g_pFullFileSystem->IsDirectory( tmpDir ) )
		g_pFullFileSystem->CreateDirHierarchy( tmpDir );
#endif
}
void DestroyDirectoryStrings()
{
}
const char *GetWorkingDirectory()
{
	return _wd;
}
const char *GetAccountName()
{
	return _acc;
}
const char *GetGamePath()
{
	return _gamePath;
}
const char *GetBinaryPath()
{
	return _compilePath;
}
const char *GetBinaryPath_Local()
{
	return _compilePath_Local;
}
const char *GetCanvasDirectory()
{
	return _canvasDir;
}
const char *GetShaderSourceDirectory()
{
	return _shaderSource;
}
const char *GetDumpDirectory()
{
	return _DumpFiles;
}
const char *GetUserFunctionDirectory()
{
	return _uFuncs;
}
const char *GetEditorRootDirectory()
{
	return _seditRoot;
}
#ifdef SHADER_EDITOR_DLL_SWARM
const char *GetSwarmShaderDirectory()
{
	return _swarmShaderDir;
}
#endif

void ComposeShaderName( GenericShaderData *data, bool bPS, bool bExtension, char *out, int maxbuf, bool bPosOverride )
{
	const bool bPreview = data->IsPreview();
	const bool sm2b = data->shader->iShaderModel == SM_20B;
	Q_snprintf( out, maxbuf, "%s_%s%s%s%s", data->name,
		(bPosOverride?"pos_":""),
		(bPreview?"preview_":""),
		((bPS)?
			(sm2b?"ps20b":"ps30"):
			(sm2b?"vs20":"vs30")
		),
		(bExtension?".fxc":"")
	);
	Q_FixSlashes( out );
	//Q_strlower( out );
}
void ComposeShaderPath( GenericShaderData *data, bool bPS, bool bExtension, char *out, int maxbuf, bool bPosOverride )
{
	char sname[MASTERPATH_MAXLEN];
	ComposeShaderName( data, bPS, bExtension, sname, sizeof( sname ), bPosOverride );
	Q_snprintf( out, maxbuf, "%s/%s", GetShaderSourceDirectory(), sname );
	Q_FixSlashes( out );
}


void ComposeShaderName_Compiled( GenericShaderData *data, bool bPS, bool bExtension, char *out, int maxbuf, bool bPosOverride )
{
	const bool bPreview = data->IsPreview();
	const bool sm2b = data->shader->iShaderModel == SM_20B;
	Q_snprintf( out, maxbuf, "%s_%s%s%s%s", data->name,
		(bPosOverride?"pos_":""),
		(bPreview?"preview_":""),
		((bPS)?
			(sm2b?"ps20b":"ps30"):
			(sm2b?"vs20":"vs30")
		),
		(bExtension?".vcs":"")
	);
	Q_FixSlashes( out );
	//Q_strlower( out );
}
void ComposeShaderPath_Compiled( GenericShaderData *data, bool bPS, bool bExtension, char *out, int maxbuf, bool bPosOverride )
{
	char sname[MASTERPATH_MAXLEN];
	ComposeShaderName_Compiled( data, bPS, bExtension, sname, sizeof( sname ), bPosOverride );
	Q_snprintf( out, maxbuf, "%s/shaders/fxc/%s", GetShaderSourceDirectory(), sname );
	Q_FixSlashes( out );
}

void ComposeShaderPath_CompiledEngine( GenericShaderData *data, bool bPS, bool bExtension, char *out, int maxbuf, bool bPosOverride )
{
	char sname[MASTERPATH_MAXLEN];
	ComposeShaderName_Compiled( data, bPS, bExtension, sname, sizeof( sname ), bPosOverride );
#if 0 //def SHADER_EDITOR_DLL_SWARM
	Q_snprintf( out, maxbuf, "%s/%s", GetSwarmShaderDirectory(), sname );

	if ( !bPosOverride )
		ShaderNameToSwarmUnique( out, maxbuf );
#else
	Q_snprintf( out, maxbuf, "%s/shaders/fxc/%s", GetGamePath(), sname );
#endif
	Q_FixSlashes( out );
}


bool bIsHex( const char &c )
{
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

#define pFnFilenameMask( x ) bool(* x )( const char *pszFilename )
#define pFnFilenameMask_Declare( x ) bool x( const char *pszFilename )

pFnFilenameMask_Declare( CheckPreviewMask )
{
	int len = Q_strlen( pszFilename );
	if ( len > 7 )
	{
		bool bFileOkay = true;
		for ( int i = 0; i < 6; i++ )
			if ( !bIsHex(pszFilename[i]) )
				bFileOkay = false;
		if ( pszFilename[6] != '_' )
			bFileOkay = false;
		return bFileOkay;
	}

	return false;
}

void ListFiles( const char *dir, CUtlVector< char* > &hFiles, const char *pszExtension = NULL, pFnFilenameMask( pCheck ) = NULL )
{
	int dirlen = Q_strlen( dir );

	FileFindHandle_t _handle;
	char tmp[MASTERPATH_MAXLEN];
//	Q_snprintf( tmp, MAX_PATH, "%s\\*_preview_*.*", dir );
	Q_snprintf( tmp, sizeof( tmp ), "%s\\*.%s", dir, (pszExtension ? pszExtension : "*") );

	char curFile[ MASTERPATH_MAXLEN ] = { '\0' };
	const char *tmpFile = g_pFullFileSystem->FindFirst( tmp, &_handle );
	if ( tmpFile )
		Q_strncpy( curFile, tmpFile, sizeof(curFile) );

	while ( curFile[0] )
	{
		bool bAddFile = pCheck ? pCheck( curFile ) : true;

		if ( bAddFile )
		{
			int len = Q_strlen( curFile );
			len = len + dirlen + 2;
			char *Addme = new char [len];
			Q_snprintf( Addme, len, "%s\\%s", dir, curFile );
			hFiles.AddToTail( Addme );
		}

		const char *tmpFile = g_pFullFileSystem->FindNext( _handle );
		if ( tmpFile )
			Q_strncpy( curFile, tmpFile, sizeof(curFile) );
		else
			curFile[0] = '\0';
	}

	g_pFullFileSystem->FindClose( _handle );
}


void PreviewCleanup()
{
	const bool bDBG = shaderEdit->ShouldShowPrimaryDbg();
	CUtlVector< char* >hFiles;
	if ( bDBG )
		Msg("*** cleanup start ***\n");
	char tmp[MASTERPATH_MAXLEN];
	Q_snprintf( tmp, sizeof( tmp ), "%s", GetShaderSourceDirectory() );
	ListFiles( tmp, hFiles, NULL, &CheckPreviewMask );
	Q_snprintf( tmp, sizeof( tmp ), "%s\\shaders\\fxc", GetShaderSourceDirectory() );
	ListFiles( tmp, hFiles, NULL, &CheckPreviewMask );
	Q_snprintf( tmp, sizeof( tmp ), "%s\\shaders\\fxc", GetGamePath() );
	ListFiles( tmp, hFiles, NULL, &CheckPreviewMask );
#if 0 //def SHADER_EDITOR_DLL_SWARM
	Q_snprintf( tmp, sizeof( tmp ), "%s", GetSwarmShaderDirectory() );
	ListFiles( tmp, hFiles, NULL, &CheckPreviewMask );
#endif
	if ( bDBG )
		Msg("killing %i files:\n", hFiles.Count());
	for ( int i = 0; i < hFiles.Count(); i++ )
	{
		if ( !hFiles[ i ] )
		{
			Assert( 0 );
			continue;
		}

		Q_memset( tmp, 0, sizeof( tmp ) );
		if ( *hFiles[ i ] )
		{
			if ( g_pFullFileSystem->FileExists( hFiles[ i ] ) )
			{
				g_pFullFileSystem->RemoveFile( hFiles[ i ] );
				if ( bDBG )
					Msg("deleting: %s\n", hFiles[ i ]);
			}
			else if ( g_pFullFileSystem->FullPathToRelativePath( hFiles[ i ], tmp, MASTERPATH_MAXLEN ) )
			{
				g_pFullFileSystem->RemoveFile( tmp, "MOD" );
				if ( bDBG )
					Msg("deleting: %s\n", tmp);
			}
		}
		delete [] hFiles[ i ];
	}
	hFiles.Purge();
	if ( bDBG )
		Msg("***  cleanup end  ***\n");
}


#ifdef SHADER_EDITOR_DLL_SWARM
void CopyAllFiles( const char *pPathSrc, const char *pPathDst, const char *pszExtension = NULL, const bool bNoOverride = false, const bool bSwarmShader = false )
{
	const bool bDBG = shaderEdit->ShouldShowPrimaryDbg();
	CUtlVector< char* >hFiles;

	ListFiles( pPathSrc, hFiles, pszExtension );

	char fullPathSrc[MASTERPATH_MAXLEN];
	char fullPathTarget[MASTERPATH_MAXLEN];
	const char *filename;

	for ( int i = 0; i < hFiles.Count(); i++ )
	{
		Assert( hFiles[i] != NULL );

		if ( !g_pFullFileSystem->RelativePathToFullPath( hFiles[i], "MOD", fullPathSrc, sizeof(fullPathSrc) ) )
			Q_snprintf( fullPathSrc, sizeof(fullPathSrc), "%s", hFiles[i] );

		filename = Q_UnqualifiedFileName( hFiles[i] );
		Q_snprintf( fullPathTarget, sizeof(fullPathTarget), "%s/%s", pPathDst, filename );

		if ( bSwarmShader )
			ShaderNameToSwarmUnique( fullPathTarget, sizeof(fullPathTarget) );

		if ( bNoOverride && g_pFullFileSystem->FileExists( fullPathTarget ) )
			continue;

		if ( bDBG )
			Msg( "FILE SRC: %s - FILE TARGET: %s\n", fullPathSrc, fullPathTarget );

		EngineCopy( fullPathSrc, fullPathTarget );

		delete [] hFiles[i];
	}

	hFiles.Purge();
}

void InitSwarmShaders()
{
	const bool bDBG = shaderEdit->ShouldShowPrimaryDbg();
	if ( bDBG )
		Msg("*** swarm auto copy start ***\n");

	char swarm_shaders_dst[MASTERPATH_MAXLEN];
	if ( !g_pFullFileSystem->RelativePathToFullPath( GetSwarmShaderDirectory(), "MOD", swarm_shaders_dst, sizeof(swarm_shaders_dst) ) )
		Q_snprintf( swarm_shaders_dst, sizeof(swarm_shaders_dst), "%s", GetSwarmShaderDirectory() );

	char user_shaders_src[MASTERPATH_MAXLEN];
	Q_snprintf( user_shaders_src, sizeof( user_shaders_src ), "%s\\shaders\\fxc", GetShaderSourceDirectory() );

	char editor_shaders_src[MASTERPATH_MAXLEN];
	Q_snprintf( editor_shaders_src, sizeof( editor_shaders_src ), "%s\\redistribute\\shaders", GetEditorRootDirectory() );

	CopyAllFiles( user_shaders_src, swarm_shaders_dst, "vcs", false, true );
	CopyAllFiles( editor_shaders_src, swarm_shaders_dst, "vcs" );

	char editor_compiler_src[MASTERPATH_MAXLEN];
	Q_snprintf( editor_compiler_src, sizeof( editor_compiler_src ), "%s\\redistribute\\compiler", GetEditorRootDirectory() );

	CopyAllFiles( editor_compiler_src, GetBinaryPath(), "dll", true );

	if ( bDBG )
		Msg("***  swarm auto copy end  ***\n");
}
#endif


CON_COMMAND( shaderEditor_filecleanup, "" )
{
	PreviewCleanup();
}