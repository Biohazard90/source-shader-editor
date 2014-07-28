#ifndef IPROCSHADER_H
#define IPROCSHADER_H
#ifdef _WIN32
#pragma once
#endif

#include "ivshadereditor.h"

#define AMT_PS_SAMPLERS 16
#define AMT_PS_CREG 13 //14

#define AMT_VS_SAMPLERS 4
#define AMT_VS_CREG 10

#define AMT_VMT_STATIC 16
#define AMT_VMT_MUTABLE 10

struct _clCallback
{
	_clCallback();
	~_clCallback();
	_clCallback( const _clCallback &o );

	char *name;
	pFnClCallback( func );
	int numComps;
};

void ShaderNameToSwarmUnique( char *pBuf, int maxLen );

#if defined(DEBUG) && !defined(PROCSHADER_DLL)
#include "editorcommon.h"
class HNODE
{
public:
	HNODE()
	{
		value = 0;
		AllocCheck_Alloc();
	};
	HNODE( unsigned int i )
	{
		value = i;
		AllocCheck_Alloc();
	};
	~HNODE()
	{
		AllocCheck_Free();
	};
	HNODE(const HNODE &o)
	{
		AllocCheck_Alloc();
		value = o.value;
	};

	operator unsigned int() const
	{
		return value;
	}

	inline bool operator==( const HNODE &o ){ return value == o.value; };
	inline bool operator==( unsigned int i ){ return value == i; };
	inline HNODE& operator=( const HNODE &o ){ value = o.value; return *this; };
	inline HNODE& operator=( unsigned int i ){ value = i; return *this; };
	inline HNODE& operator++(){ value++; return *this; };
	inline HNODE operator++(int){ HNODE t = *this; ++*this; return t; };
	inline HNODE& operator--(){ value--; return *this; };
	inline HNODE operator--(int){ HNODE t = *this; --*this; return t; };

private:
	unsigned int value;
};
#else
typedef unsigned int HNODE;
#endif

class KeyValues;
void UnpackMaterial( KeyValues *pKV, int recursionDepth = 0 );

enum
{
	NPSOP_WRITE_TEXCOORD_0 = 0,
	NPSOP_WRITE_TEXCOORD_1,
	NPSOP_WRITE_TEXCOORD_2,
	NPSOP_WRITE_TEXCOORD_3,
	NPSOP_WRITE_TEXCOORD_4,
	NPSOP_WRITE_TEXCOORD_5,
	NPSOP_WRITE_TEXCOORD_6,
	NPSOP_WRITE_TEXCOORD_7,
	NPSOP_WRITE_COLOR_0,
	NPSOP_WRITE_COLOR_1,

	NPSOP_WRITE_MAX,
};

enum
{
	NPSOP_CALC_ADD = 0,
	NPSOP_CALC_SUBTRACT,
	NPSOP_CALC_SWIZZLE,
	NPSOP_CALC_MULTIPLY,
	NPSOP_CALC_DIVIDE,

	NPSOP_TEXTURE_LOOKUP_2D,
	NPSOP_TEXTURE_LOOKUP_3D,

	NPSOP_SET_CONSTANT,
	NPSOP_UTILITY_DECLARE,
	NPSOP_UTILITY_ASSIGN,

	NPSOP_CALC_DOT,
	NPSOP_CALC_NORMALIZE,
	NPSOP_CALC_LERP,
	NPSOP_CALC_SMOOTHSTEP,
	NPSOP_CALC_CLAMP,
	NPSOP_CALC_LENGTH,
	NPSOP_CALC_POW,
	NPSOP_CALC_STEP,
	NPSOP_CALC_ROUND,
	NPSOP_CALC_FRAC,
	NPSOP_CALC_FLOOR,
	NPSOP_CALC_CEIL,
	NPSOP_CALC_ABS,

	NPSOP_CALC_SIN,
	NPSOP_CALC_ASIN,
	NPSOP_CALC_SINH,
	NPSOP_CALC_COS,
	NPSOP_CALC_ACOS,
	NPSOP_CALC_COSH,
	NPSOP_CALC_TAN,
	NPSOP_CALC_ATAN,
	NPSOP_CALC_TANH,
	NPSOP_CALC_ATAN2,

	NPSOP_CALC_MIN,
	NPSOP_CALC_MAX,
	NPSOP_CALC_FMOD,
	NPSOP_CALC_DEGREES,
	NPSOP_CALC_RADIANS,

	NPSOP_CALC_REFLECT,
	NPSOP_CALC_REFRACT,
	NPSOP_CALC_SATURATE,
	NPSOP_CALC_INVERT,

	NPSOP_CALC_TEXTRANSFORM,
	NPSOP_CALC_SIGN,
	NPSOP_CALC_APPEND,
	NPSOP_CALC_SQRT,
	NPSOP_CALC_CROSS,

	NPSOP_CALC_PSLIGHTING,
	NPSOP_CALC_MCOMP,

	NPSOP_CALC_LOG,
	NPSOP_CALC_LOG2,
	NPSOP_CALC_LOG10,

	NPSOP_CALC_EXP,
	NPSOP_CALC_EXP2,

	NPSOP_CALC_DDX,
	NPSOP_CALC_DDY,

	NPSOP_TEXTURE_LOOKUP_FIXED,

	NPSOP_CALC_DIST,

	NPSOP_CALC_LAST,
};

enum HLSLComboTypes
{
	HLSLCOMBO_INVALID = 0,

	HLSLCOMBO_CUSTOM,

	HLSLCOMBO_SKINNING,
	HLSLCOMBO_MORPHING,

	HLSLCOMBO_LIGHT_STATIC,
	HLSLCOMBO_LIGHT_DYNAMIC,
	HLSLCOMBO_NUM_LIGHTS,

	HLSLCOMBO_PIXELFOG,
	HLSLCOMBO_WATERFOG_TOALPHA,

	HLSLCOMBO_FLASHLIGHT_ENABLED,
	HLSLCOMBO_FLASHLIGHT_FILTER_MODE,
	HLSLCOMBO_FLASHLIGHT_DO_SHADOWS,

	HLSLCOMBO_VERTEXCOMPRESSION,
	HLSLCOMBO_MAX,
};

enum HLSLTextureSamplerModes
{
	HLSLTEX_CUSTOMPARAM = 0,

	HLSLTEX_BASETEXTURE,
	HLSLTEX_BUMPMAP,
	HLSLTEX_LIGHTMAP,
	HLSLTEX_LIGHTMAP_BUMPMAPPED,

	HLSLTEX_FRAMEBUFFER,
	HLSLTEX_ENVMAP,

	HLSLTEX_BLACK,
	HLSLTEX_WHITE,
	HLSLTEX_GREY,

	HLSLTEX_FLASHLIGHT_COOKIE,
	HLSLTEX_FLASHLIGHT_DEPTH,
	HLSLTEX_FLASHLIGHT_RANDOM,

	HLSLTEX_MORPH,
	HLSLTEX_,
};

#define HLSLTEXSAM_LAST_USABLE HLSLTEX_GREY
//#define ISTEXTURESAMPLERLOCKED(x) ( x == HLSLTEX_FLASHLIGHT_COOKIE || x == HLSLTEX_FLASHLIGHT_RANDOM || x == HLSLTEX_FLASHLIGHT_DEPTH )
#define IS_TEXTURE_SAMPLER_USING_CUSTOM_TEXTURE( x ) ( x == HLSLTEX_CUSTOMPARAM ||\
	x == HLSLTEX_BASETEXTURE ||\
	x == HLSLTEX_BUMPMAP )

enum
{
	CMATRIX_VIEW = 0,
	CMATRIX_PROJ,
	CMATRIX_VIEWPROJ,

	CMATRIX_VIEW_INV,
	CMATRIX_PROJ_INV,
	CMATRIX_VIEWPROJ_INV,

	CMATRIX_LAST,
};

enum HLSLEnvironmentConstants
{
	HLSLENV_TIME = 0,

	HLSLENV_VIEW_ORIGIN,
	HLSLENV_VIEW_FWD,
	HLSLENV_VIEW_RIGHT,
	HLSLENV_VIEW_UP,
	HLSLENV_VIEW_WORLDDEPTH,
	HLSLENV_PIXEL_SIZE,
	HLSLENV_FOG_PARAMS,

	// helper constants
	HLSLENV_STUDIO_LIGHTING_VS,
	HLSLENV_STUDIO_LIGHTING_PS,
	HLSLENV_STUDIO_MORPHING,

	HLSLENV_FLASHLIGHT_VPMATRIX,
	HLSLENV_FLASHLIGHT_DATA,
	HLSLENV_CUSTOM_MATRIX,

	HLSLENV_SMART_CALLBACK,
	HLSLENV_SMART_VMT_STATIC,
	HLSLENV_SMART_VMT_MUTABLE,
	HLSLENV_SMART_RANDOM_FLOAT,

	HLSLENV_LIGHTMAP_RGB,

	HLSLENV_MAX,
};

// true when using custom hlsl code printing
#define HLSLENV_IS_MANUAL_CONST( x ) ( x >= HLSLENV_STUDIO_LIGHTING_VS )
// true when using smarthelper string on uniquify
#define HLSLENV_IS_SMART_CONST( x ) ( x >= HLSLENV_SMART_CALLBACK && x <= HLSLENV_SMART_RANDOM_FLOAT )
// true if const uses custom register
#define HLSLENV_SHOULD_COUNT_CONST( x, IsPS ) ( x <= HLSLENV_FOG_PARAMS ||\
											( x == HLSLENV_FLASHLIGHT_VPMATRIX && !IsPS ) ||\
											x == HLSLENV_SMART_CALLBACK ||\
											x == HLSLENV_SMART_VMT_STATIC ||\
											x == HLSLENV_SMART_VMT_MUTABLE ||\
											x == HLSLENV_SMART_RANDOM_FLOAT ||\
											x == HLSLENV_CUSTOM_MATRIX ||\
											x == HLSLENV_LIGHTMAP_RGB)

// autocopy data from viewsetup
#define HLSLENV_AUTOCOPYCONST_FIRST HLSLENV_VIEW_ORIGIN
#define HLSLENV_AUTOCOPYCONST_LAST HLSLENV_VIEW_WORLDDEPTH

// map it to an actual hlsl register
int RemapEnvironmentConstant( bool bPixelShader, int iNormalized );

#ifndef PROCSHADER_DLL
#include "interface.h"
#else
#include "../public/tier1/interface.h"
#endif


enum
{
	SM_20B = 0,
	SM_30,
};
enum
{
	CULLMODE_CW = 0,
	CULLMODE_CCW,
	//CULLMODE_DOUBLE,
	CULLMODE_NONE,
};
enum
{
	ABLEND_NONE = 0,
	ABLEND_SIMPLE,
	ABLEND_SIMPLE_INVERTED,
	ABLEND_ALPHATEST,
	ABLEND_ADDITIVE,
	ABLEND_ALPHA2COVERAGE,
};
#define ABLEND_IS_TRANSLUCENT( x ) ( x >= ABLEND_SIMPLE && x <= ABLEND_SIMPLE_INVERTED ||\
									x == ABLEND_ADDITIVE )
enum
{
	DEPTHTEST_OFF = 0,
	DEPTHTEST_NORMAL,
};
enum
{
	DEPTHWRITE_OFF = 0,
	DEPTHWRITE_NORMAL,
};

enum
{
	PROCS_TEX_TYPE_DEFAULT = 0,
	PROCS_TEX_TYPE_CUBEMAP,
	PROCS_TEX_TYPE_NORMALMAP,
};

struct __funcParamSetup
{
public:
	__funcParamSetup();
	~__funcParamSetup();
	__funcParamSetup( const __funcParamSetup &o );

	const bool operator==( const __funcParamSetup &o ) const
	{
		return //bOutput == o.bOutput &&
			iFlag == o.iFlag;
	};
	const bool operator!=( const __funcParamSetup &o ) const
	{ return !( (*this) == o ); };

	int iFlag;
	char *pszName;
	bool bOutput;

	const char *GetSafeName( int num );
};

struct SimpleTexture
{
public:
	SimpleTexture();
	~SimpleTexture();
	SimpleTexture( const SimpleTexture &o );

	// custom texture to load
	char *szTextureName;

	// custom parameter - uniquify this
	char *szParamName;
	// use this texture when param undefined in vmt
	char *szFallbackName;
	// sampler index - defined on uniquify
	int iSamplerIndex;
	// texture mode, bind standard or from param?
	int iTextureMode;
	// do we need a cubemap lookup?
	bool bCubeTexture;
	bool bSRGB;

	CUtlVector< HNODE* >m_hTargetNodes;
};

struct SimpleCombo
{
public:
	SimpleCombo();
	~SimpleCombo();
	SimpleCombo( const SimpleCombo &o );

	char *name;
	int iComboType;
	bool bStatic;
	int min;
	int max;

	bool bInPreviewMode;

	int GetAmt()
	{
		Assert( max >= min );
		return (max - min) + 1;
	};
};

struct SimpleEnvConstant
{
public:
	SimpleEnvConstant();
	~SimpleEnvConstant();
	SimpleEnvConstant( const SimpleEnvConstant &o );

	int iEnvC_ID;
	int iHLSLRegister;
	int iConstSize;

	char *szSmartHelper;
	int iSmartNumComps;
	float flSmartDefaultValues[4];

// only valid in shader lib *AFTER* shader init
	int iFastLookup;
};

// only valid for compilation, never saved to dumps!
struct SimpleArray
{
public:
	SimpleArray();
	~SimpleArray();
	SimpleArray( const SimpleArray &o );

	Vector4D *vecData;
	int iSize_X;
	int iSize_Y;

	int iNumComps;

	HNODE iIndex;
};

// only valid for compilation, never saved to dumps!
struct SimpleFunction
{
public:
	SimpleFunction();
	~SimpleFunction();
	SimpleFunction( const SimpleFunction &o );

	bool IsInline();

	char *szFuncName;
	char *szFilePath;

	char *szCode_Global;
	char *szCode_Body;

	CUtlVector< __funcParamSetup* >hParams;
};

struct IdentifierLists_t
{
public:
	IdentifierLists_t();
	~IdentifierLists_t();
	IdentifierLists_t( const IdentifierLists_t &o );

	CUtlVector< SimpleTexture* > hList_Textures;
	CUtlVector< SimpleCombo* > hList_Combos;
	CUtlVector< SimpleEnvConstant* > hList_EConstants;
	CUtlVector< SimpleArray* > hList_Arrays;
	CUtlVector< SimpleFunction* > hList_Functions;
	
	int inum_DynamicCombos;
};

struct BasicShaderCfg_t
{
	BasicShaderCfg_t();
	BasicShaderCfg_t( const BasicShaderCfg_t &o );
	~BasicShaderCfg_t();

	char *CanvasName; // only for preloaded shaders!
	char *Filename;

	char *ProcVSName;
	char *ProcPSName;

	char dumpversion[16];

	int iShaderModel;
	int iCullmode;
	int iAlphablendmode;
	float flAlphaTestRef;
	int iDepthtestmode;
	int iDepthwritemode;
	bool bsRGBWrite;

	int iVFMT_flags;
	int iVFMT_numTexcoords;
	int iVFMT_numUserData;
	int iVFMT_texDim[3];

	bool bVertexLighting;
	bool bRefractionSupport;
	bool bPreviewMode;

	//CUtlVector< SimpleCombo* > hList_Combos_VS;
	//CUtlVector< SimpleCombo* > hList_Combos_PS;

	IdentifierLists_t *pVS_Identifiers;
	IdentifierLists_t *pPS_Identifiers;
};

class IVPPEHelper;

class IVProcShader : public IBaseInterface
{
public:
	virtual bool Init( CreateInterfaceFn appSystemFactory, IVPPEHelper *pPPEHelper = NULL ) = 0;
	virtual void Shutdown() = 0;

	virtual void *SwapShaderSystem( void *_data, const int &index ) = 0;
	virtual void UpdateEnvironmentData( int iEnvC, float *_fl4 ) = 0;

	virtual void SetNormalizedPuzzleDelta( float d ) = 0;

	virtual void AddPreloadShader( void *data ) = 0;
	virtual int GetNumPreloadShaders() = 0;
	virtual int FindPreloadShader( const char *name ) = 0;
	virtual void *GetPreloadShader( const int idx ) = 0;
	virtual void *GetAndRemovePreloadShader( const int idx ) = 0;
	virtual void *SwapPreloadShader( const int idx, void *_data ) = 0;

	virtual void LinkCallbacks( CUtlVector< _clCallback* > *hList ) = 0;
};

#define PPEINLINE_PARAM_KEY __PPE_INLINE

class IVPPEHelper : public IBaseInterface
{
public:

	virtual KeyValues *GetInlineMaterial( const char *szmatName ) = 0;
	virtual void DestroyKeyValues( KeyValues *pKV ) = 0;
};

#ifndef PROCSHADER_DLL
extern IVProcShader *gProcShaderCTRL;

class CPPEHelper;
extern CPPEHelper *gPPEHelper;
#else
class ProcShaderInterface;
extern ProcShaderInterface *gProcShaderCTRL;

extern IVPPEHelper *gPPEHelper;
#endif

#define PROCSHADER_INTERFACE_VERSION "ProceduralShaderImplementation_001"

#endif