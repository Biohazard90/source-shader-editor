
#ifdef CLIENT_DLL
#include "cbase.h"
#include "filesystem.h"
#else
#include "procshader/shaderincludes.h"
#include "ProcShaderInterface.h"
#endif

#include "IVProcShader.h"

#ifndef AllocCheck_Alloc
#define AllocCheck_Alloc() ((void)NULL)
#endif
#ifndef AllocCheck_Free
#define AllocCheck_Free() ((void)NULL)
#endif
#ifndef AllocCheck_AllocS
#define AllocCheck_AllocS(x) ((void)NULL)
#endif
#ifndef AllocCheck_FreeS
#define AllocCheck_FreeS(x) ((void)NULL)
#endif


void AddDataToMaterial( KeyValues *pMat, KeyValues *pData, int recursionDepth = 0 )
{
	UnpackMaterial( pData, recursionDepth );

	for ( KeyValues *pValue = pData->GetFirstValue(); pValue != NULL; pValue = pValue->GetNextValue() )
	{
		const char *pszParam = pValue->GetName();
		const char *pszValue = pValue->GetString();

		pMat->SetString( pszParam, pszValue );
	}
}

void UnpackMaterial( KeyValues *pKV, int recursionDepth )
{
	Assert( pKV );

	if ( recursionDepth > 99 )
	{
		Warning( "ABORTING MATERIAL UNPACKING OF %s - INFINITELY RECURSIVE!\n", pKV->GetName() );
		return;
	}

	const bool bIsPatch = !Q_stricmp( pKV->GetName(), "patch" );

	for ( KeyValues *pSub = pKV->GetFirstSubKey(); pSub != NULL; pSub = pSub->GetNextKey() )
	{
		const char *pszName = pSub->GetName();

		if ( !Q_stricmp( pszName, "include" ) ) // a string
		{
			const char *pFile = pSub->GetString();
			if ( !pFile || !*pFile )
				continue;

			KeyValues *pInlcude = new KeyValues( "include" );
			if ( pInlcude->LoadFromFile( g_pFullFileSystem, pFile ) )
			{
				AddDataToMaterial( pKV, pInlcude, recursionDepth );
				if ( bIsPatch ) // recursive patches!
					pKV->SetName( pInlcude->GetName() );
			}

			pInlcude->deleteThis();
		}
		else if ( !Q_stricmp( pszName, "replace" ) || !Q_stricmp( pszName, "insert" ) ) // a subkey
		{
			for ( KeyValues *pValue = pSub->GetFirstValue(); pValue != NULL; pValue = pValue->GetNextValue() )
			{
				pKV->SetString( pValue->GetName(), pValue->GetString() );
			}
		}
	}
}

_clCallback::_clCallback()
{
	name = NULL;
	func = NULL;
	numComps = 1;
}
_clCallback::~_clCallback()
{
	delete [] name;
}
_clCallback::_clCallback( const _clCallback &o )
{
	name = NULL;
	if ( o.name != NULL && Q_strlen( o.name ) )
	{
		name = new char [ Q_strlen( o.name ) + 1 ];
		Q_strcpy( name, o.name );
	}
	func = o.func;
	numComps = o.numComps;
}

int RemapEnvironmentConstant( bool bPixelShader, int iNormalized )
{
	if ( iNormalized < 0 )
		return -1;

	int offset = 0;
	Assert( iNormalized >= 0 );
	if ( !bPixelShader )
	{
		offset = 48;
		Assert( iNormalized <= 9 );
	}
	else
	{
#if 0
		if ( iNormalized <= 3 )
			offset = 0;
		else if ( iNormalized <= 8 )
			offset = 6;
		else if ( iNormalized <= 9 )
			offset = 6 + 4;
		else if ( iNormalized <= 10 )
			offset = 6 + 4 + 6;
		Assert( iNormalized <= 8 );
#else
		offset = 16;
		Assert( iNormalized <= 13 );
#endif
	}
	return offset + iNormalized;
}

void ShaderNameToSwarmUnique( char *pBuf, int maxLen )
{
	if ( !pBuf || !*pBuf || maxLen < 1 )
		return;

	char tmp[MAX_PATH*4];
	Q_snprintf( tmp, sizeof( tmp ), "%s", pBuf );

	const char *pLastUnderscore = pBuf + Q_strlen(pBuf) - 1;
	while ( pLastUnderscore > pBuf && *pLastUnderscore != '_' )
		pLastUnderscore--;

	if ( pLastUnderscore < pBuf || *pLastUnderscore != '_' )
		return;

	pLastUnderscore++;
	int offset = pLastUnderscore - pBuf;

	if ( offset >= sizeof( tmp ) || offset < 0 )
		return;

	tmp[offset] = '\0';
	Q_strcat( tmp, "mod_", sizeof( tmp ) );

	if ( *pLastUnderscore )
		Q_strcat( tmp, pLastUnderscore, sizeof( tmp ) );

	Q_snprintf( pBuf, maxLen, "%s", tmp );
}

BasicShaderCfg_t::BasicShaderCfg_t()
{
	iShaderModel = SM_30;
	iCullmode = CULLMODE_CW;
	iAlphablendmode = ABLEND_NONE;
	flAlphaTestRef = 0.5f;
	iDepthtestmode = DEPTHTEST_NORMAL;
	iDepthwritemode = DEPTHWRITE_NORMAL;
	bsRGBWrite = false;
	bPreviewMode = false;

	iVFMT_flags = VERTEX_POSITION;
	iVFMT_numTexcoords = 1;
	iVFMT_numUserData = 0;
	iVFMT_texDim[0] = 2;
	iVFMT_texDim[1] = 2;
	iVFMT_texDim[2] = 2;

	ProcVSName = NULL;
	ProcPSName = NULL;
	CanvasName = NULL;
	Filename = NULL;
	dumpversion[0] = '\0';
	//pVS_Identifiers = pPS_Identifiers = NULL;
	pVS_Identifiers = new IdentifierLists_t();
	pPS_Identifiers = new IdentifierLists_t();

	bVertexLighting = false;
	bRefractionSupport = false;

	AllocCheck_Alloc();
}

BasicShaderCfg_t::BasicShaderCfg_t( const BasicShaderCfg_t &o )
{
	ProcVSName = NULL;
	ProcPSName = NULL;
	CanvasName = NULL;
	Filename = NULL;

	if ( o.ProcVSName )
	{
		int len = Q_strlen( o.ProcVSName ) + 1;
		ProcVSName = new char[ len ];
		Q_snprintf( ProcVSName, len, "%s", o.ProcVSName );
	}
	if ( o.ProcPSName )
	{
		int len = Q_strlen( o.ProcPSName ) + 1;
		ProcPSName = new char[ len ];
		Q_snprintf( ProcPSName, len, "%s", o.ProcPSName );
	}
	if ( o.CanvasName )
	{
		int len = Q_strlen( o.CanvasName ) + 1;
		CanvasName = new char[ len ];
		Q_snprintf( CanvasName, len, "%s", o.CanvasName );
	}
	if ( o.Filename )
	{
		int len = Q_strlen( o.Filename ) + 1;
		Filename = new char[ len ];
		Q_snprintf( Filename, len, "%s", o.Filename );
	}

	Q_strcpy( dumpversion, o.dumpversion );

	iShaderModel = o.iShaderModel;
	iCullmode = o.iCullmode;
	iAlphablendmode = o.iAlphablendmode;
	flAlphaTestRef = o.flAlphaTestRef;
	iDepthtestmode = o.iDepthtestmode;
	iDepthwritemode = o.iDepthwritemode;
	bsRGBWrite = o.bsRGBWrite;
	bPreviewMode = o.bPreviewMode;

	iVFMT_flags = o.iVFMT_flags;
	iVFMT_numTexcoords = o.iVFMT_numTexcoords;
	iVFMT_numUserData = o.iVFMT_numUserData;
	Q_memcpy( iVFMT_texDim, o.iVFMT_texDim, sizeof(int) * 3 );

	pVS_Identifiers = NULL;
	pPS_Identifiers = NULL;

	bVertexLighting = o.bVertexLighting;
	bRefractionSupport = o.bRefractionSupport;

	if ( o.pVS_Identifiers )
		pVS_Identifiers = new IdentifierLists_t(*o.pVS_Identifiers);
	if ( o.pPS_Identifiers )
		pPS_Identifiers = new IdentifierLists_t(*o.pPS_Identifiers);

	AllocCheck_Alloc();
}

BasicShaderCfg_t::~BasicShaderCfg_t()
{
	delete [] ProcVSName;
	delete [] ProcPSName;
	delete [] CanvasName;
	delete [] Filename;

	delete pVS_Identifiers;
	delete pPS_Identifiers;

	AllocCheck_Free();
}

SimpleCombo::SimpleCombo()
{
	bStatic = false;
	bInPreviewMode = false;
	min = 0;
	max = 1;
	name = NULL;
	iComboType = 0;

	AllocCheck_Alloc();
}
SimpleCombo::~SimpleCombo()
{
	delete [] name;

	AllocCheck_Free();
}
SimpleCombo::SimpleCombo(const SimpleCombo &o)
{
	name = NULL;
	if ( o.name )
	{
		int len = Q_strlen( o.name ) + 1;
		name = new char[ len ];
		Q_snprintf( name, len, "%s", o.name );
	}
	min = o.min;
	max = o.max;
	bStatic = o.bStatic;
	iComboType = o.iComboType;
	bInPreviewMode = o.bInPreviewMode;

	AllocCheck_Alloc();
}
SimpleTexture::SimpleTexture()
{
	szTextureName = NULL;
	szParamName = NULL;
	szFallbackName = NULL;
	iSamplerIndex = 0;
	iTextureMode = 0;
	bCubeTexture = false;
	bSRGB = false;

	AllocCheck_Alloc();
}
SimpleTexture::~SimpleTexture()
{
	if ( szTextureName != NULL )
		AllocCheck_FreeS( "szTextureName" );

	delete [] szTextureName;
	delete [] szParamName;
	delete [] szFallbackName;

	m_hTargetNodes.PurgeAndDeleteElements();

	AllocCheck_Free();
}
SimpleTexture::SimpleTexture( const SimpleTexture &o )
{
	iSamplerIndex = o.iSamplerIndex;
	iTextureMode = o.iTextureMode;
	bCubeTexture = o.bCubeTexture;
	bSRGB = o.bSRGB;

	if ( o.szTextureName )
	{
		int len = Q_strlen( o.szTextureName ) + 1;
		szTextureName = new char[ len ];
		Q_snprintf( szTextureName, len, "%s", o.szTextureName );

		AllocCheck_AllocS( "szTextureName" );
	}
	else
		szTextureName = NULL;
	if ( o.szParamName )
	{
		int len = Q_strlen( o.szParamName ) + 1;
		szParamName = new char[ len ];
		Q_snprintf( szParamName, len, "%s", o.szParamName );
	}
	else
		szParamName = NULL;
	if ( o.szFallbackName )
	{
		int len = Q_strlen( o.szFallbackName ) + 1;
		szFallbackName = new char[ len ];
		Q_snprintf( szFallbackName, len, "%s", o.szFallbackName );
	}
	else
		szFallbackName = NULL;

	for ( int i = 0; i < o.m_hTargetNodes.Count(); i++ )
		m_hTargetNodes.AddToTail( new HNODE( *o.m_hTargetNodes[i] ) );

	AllocCheck_Alloc();
}

SimpleEnvConstant::SimpleEnvConstant()
{
	iEnvC_ID = HLSLENV_TIME;
	iHLSLRegister = 0;
	iConstSize = 1;
	szSmartHelper = NULL;
	iFastLookup = -1;
	iSmartNumComps = 3;
	Q_memset( flSmartDefaultValues, 0, sizeof( flSmartDefaultValues ) );

	AllocCheck_Alloc();
}
SimpleEnvConstant::~SimpleEnvConstant()
{
	delete [] szSmartHelper;

	AllocCheck_Free();
}
SimpleEnvConstant::SimpleEnvConstant( const SimpleEnvConstant &o )
{
	szSmartHelper = NULL;
	if ( o.szSmartHelper && Q_strlen( o.szSmartHelper ) )
	{
		szSmartHelper = new char[ Q_strlen( o.szSmartHelper ) + 1 ];
		Q_strcpy( szSmartHelper, o.szSmartHelper );
	}

	iEnvC_ID = o.iEnvC_ID;
	iHLSLRegister = o.iHLSLRegister;
	iConstSize = o.iConstSize;
	iFastLookup = o.iFastLookup;
	iSmartNumComps = o.iSmartNumComps;
	Q_memcpy( flSmartDefaultValues, o.flSmartDefaultValues, sizeof( flSmartDefaultValues ) );

	AllocCheck_Alloc();
}
SimpleArray::SimpleArray()
{
	vecData = NULL;
	iSize_X = 0;
	iSize_Y = 0;
	iNumComps = 0;
	iIndex = 0;

	AllocCheck_Alloc();
}
SimpleArray::SimpleArray( const SimpleArray &o )
{
	iSize_X = o.iSize_X;
	iSize_Y = o.iSize_Y;
	iNumComps = o.iNumComps;
	iIndex = o.iIndex;
	vecData = NULL;

	const int numComps = iSize_X * iSize_Y;
	if ( numComps > 0 )
	{
		vecData = new Vector4D[ numComps ];
		Q_memcpy( vecData, o.vecData, sizeof(Vector4D) * numComps );
	}

	AllocCheck_Alloc();
}
SimpleArray::~SimpleArray()
{
	delete [] vecData;

	AllocCheck_Free();
}

SimpleFunction::SimpleFunction()
{
	szFuncName = NULL;
	szFilePath = NULL;
	szCode_Global = NULL;
	szCode_Body = NULL;
}
SimpleFunction::~SimpleFunction()
{
	delete [] szFuncName;
	delete [] szFilePath;
	delete [] szCode_Global;
	delete [] szCode_Body;
	hParams.PurgeAndDeleteElements();
}
SimpleFunction::SimpleFunction( const SimpleFunction &o )
{
	if ( o.szFuncName && *o.szFuncName )
	{
		int len = Q_strlen( o.szFuncName ) + 1;
		szFuncName = new char[ len ];
		Q_strcpy( szFuncName, o.szFuncName );
	}
	else
		szFuncName = NULL;

	if ( o.szFilePath && *o.szFilePath )
	{
		int len = Q_strlen( o.szFilePath ) + 1;
		szFilePath = new char[ len ];
		Q_strcpy( szFilePath, o.szFilePath );
	}
	else
		szFilePath = NULL;

	if ( o.szCode_Global && *o.szCode_Global )
	{
		int len = Q_strlen( o.szCode_Global ) + 1;
		szCode_Global = new char[ len ];
		Q_strcpy( szCode_Global, o.szCode_Global );
	}
	else
		szCode_Global = NULL;

	if ( o.szCode_Body && *o.szCode_Body )
	{
		int len = Q_strlen( o.szCode_Body ) + 1;
		szCode_Body = new char[ len ];
		Q_strcpy( szCode_Body, o.szCode_Body );
	}
	else
		szCode_Body = NULL;

	for ( int i = 0; i < o.hParams.Count(); i++ )
		hParams.AddToTail( new __funcParamSetup( *o.hParams[i] ) );
}
bool SimpleFunction::IsInline()
{
	return !szFilePath || !Q_strlen(szFilePath);
}

__funcParamSetup::__funcParamSetup()
{
	iFlag = 1; //::HLSLVAR_FLOAT1;
	pszName = NULL;
	bOutput = false;
}
__funcParamSetup::~__funcParamSetup()
{
	if ( pszName != NULL )
		delete [] pszName;
}
__funcParamSetup::__funcParamSetup( const __funcParamSetup &o )
{
	iFlag = o.iFlag;
	pszName = NULL;
	bOutput = o.bOutput;

	if ( o.pszName != NULL )
	{
		pszName = new char[ Q_strlen( o.pszName ) + 1 ];
		Q_strcpy( pszName, o.pszName );
	}
}
const char *__funcParamSetup::GetSafeName( int num )
{
	if ( !pszName )
	{
		char _name[32];
		Q_snprintf( _name, sizeof(_name), "var_%02i", num );
		int len = Q_strlen( _name ) + 1;
		pszName = new char[ len ];
		Q_strcpy( pszName, _name );
	}
	return pszName;
}

IdentifierLists_t::IdentifierLists_t()
{
	inum_DynamicCombos = 1;

	AllocCheck_Alloc();
}
IdentifierLists_t::~IdentifierLists_t()
{
	hList_Combos.PurgeAndDeleteElements();
	hList_Textures.PurgeAndDeleteElements();
	hList_EConstants.PurgeAndDeleteElements();
	hList_Arrays.PurgeAndDeleteElements();
	hList_Functions.PurgeAndDeleteElements();

	AllocCheck_Free();
}
IdentifierLists_t::IdentifierLists_t( const IdentifierLists_t &o )
{
	for ( int i = 0; i < o.hList_Combos.Count(); i++ )
		hList_Combos.AddToTail( new SimpleCombo( *o.hList_Combos[ i ] ) );

	for ( int i = 0; i < o.hList_Textures.Count(); i++ )
		hList_Textures.AddToTail( new SimpleTexture( *o.hList_Textures[ i ] ) );

	for ( int i = 0; i < o.hList_EConstants.Count(); i++ )
		hList_EConstants.AddToTail( new SimpleEnvConstant( *o.hList_EConstants[ i ] ) );

	for ( int i = 0; i < o.hList_Arrays.Count(); i++ )
		hList_Arrays.AddToTail( new SimpleArray( *o.hList_Arrays[ i ] ) );

	for ( int i = 0; i < o.hList_Functions.Count(); i++ )
		hList_Functions.AddToTail( new SimpleFunction( *o.hList_Functions[ i ] ) );

	inum_DynamicCombos = o.inum_DynamicCombos;

	AllocCheck_Alloc();
}

//CUtlVector< SimpleCombo* > hList_Combos;