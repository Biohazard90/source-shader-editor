#ifndef EDITORCOMMON_H
#define EDITORCOMMON_H

#pragma warning( default: 4800 ) // enable: forcing value to bool 'true' or 'false' (performance warning)
#pragma warning( disable: 4005 ) // disable: macro redefinition

//#if DEBUG
//#include <typeinfo.h>
//class DbgOutput
//{
//public:
//	DbgOutput();
//	~DbgOutput();
//
//	void DbgOps( const char *pname, bool bDealloc );
//
//private:
//	void OnAllocated( const char *pname );
//	void OnFreed( const char *pname );
//};
//extern DbgOutput __dbgOutput;
//
//#define AllocCheck_Alloc() __dbgOutput.DbgOps( typeid(this).name(), false )
//#define AllocCheck_Free() __dbgOutput.DbgOps( typeid(this).name(), true )
//#define AllocCheck_AllocS(x) __dbgOutput.DbgOps( x, false )
//#define AllocCheck_FreeS(x) __dbgOutput.DbgOps( x, true )
//#else
#define AllocCheck_Alloc() ((void)NULL)
#define AllocCheck_Free() ((void)NULL)
#define AllocCheck_AllocS(x) ((void)NULL)
#define AllocCheck_FreeS(x) ((void)NULL)
//#endif

float GetEditorTime();
void EngineCopy( const char *a, const char *b );

#include "exported_utilities.h"

#ifdef SHADER_EDITOR_DLL_SWARM

#include "vgui_surfacelib/ifontsurface.h"

void VCRHook_WaitForSingleObject(
	__in HANDLE hHandle,
	__in DWORD dwMilliseconds
	);
#elif SHADER_EDITOR_DLL_2006

class CMatRenderContextPtr
{
public:
	CMatRenderContextPtr( IMaterialSystem *pMat )
	{
		m = pMat;
	};

	operator IMaterialSystem*() {
		return m;
	};

	IMaterialSystem* operator->() {
		return m;
	};

	const IMaterialSystem* operator->() const {
		return m;
	};

private:
	IMaterialSystem *m;
};

#define IMatRenderContext IMaterialSystem

#endif

#include "const.h"
#include "vgui_controls/controls.h"
#include "tier0/fasttimer.h"
#include "procshader/cpp_shader_constant_register_map.h"

#include "vgui_controls/utilities.h"

#include "dll_init.h"

class vgui::Panel;
class CJack;
class CBaseNode;
class CNodeView;
class CSolveThread;

class CHLSL_Var;
class CHLSL_SolverBase;

struct GenericShaderData;

#define SHOW_SEDIT_ERRORS ( DEBUG )

#define MAXTARGC 512

//void SetupVguiTex( int &var, const char *tex );

#define NODE_DRAW_SHADOW_DELTA 5
#define NODE_DRAW_COLOR_SHADOW Color( 0, 0, 0, 50 )

#define DRAWCOLOR_ERRORLEVEL_UNDEFINED Color( 140, 140, 160, 255 )
#define DRAWCOLOR_ERRORLEVEL_FAIL Color( 128, 0, 0, 255 )

#define DRAWCOLOR_BRIDGE_TMP_COL_ORANGE Color( 240, 160, 0, 255 )
#define DRAWCOLOR_BRIDGE_TMP_COL_TRANS Color( 0, 0, 0, 196 )

#define TOKENCHANNELS_SUPPORTED_COLOR Color( 96, 96, 0, 128 )

#define JACK_COLOR_NORM Color( 15, 15, 15, 255 )
#define JACK_COLOR_R Color( 200, 0, 0, 255 )
#define JACK_COLOR_G Color( 0, 200, 0, 255 )
#define JACK_COLOR_B Color( 0, 0, 200, 255 )
#define JACK_COLOR_A Color( 200, 200, 200, 255 )
#define JACK_COLOR_PPMASTER Color( 50, 180, 60, 255 )

#define JACK_COLOR_NAME Color( 210, 210, 220, 255 )
#define JACK_COLOR_NAME_FOCUS Color( 160, 255, 160, 255 )

#define JACK_COLOR_FOCUS Color( 240, 200, 50, 255 )

#define JACK_COLOR_DATATYPE_UNDEFINED Color( 100, 100, 100, 255 )
#define JACK_COLOR_DATATYPE Color( 160, 160, 160, 255 )
#define JACK_COLOR_DATATYPE_DARK Color( 32, 32, 32, 255 )

#define JACK_TEXT_INSET 3

#define VIEWZOOM_OUT_MAX 8.0f
#define VIEWZOOM_IN_MAX 0.001f

#define NODEVIEWHISTORY_MAXUNDO 21

#define PSIN_ERROR_WHEN_VSOUT_ERROR 0

#define ENABLE_HISTORY 1
#define DO_SOLVING 1

#define ENABLE_AUTOCOMP_PARSER 1

#define MAKE_DEFAULT_CANVAS_EDITABLE 0

Color Lerp( const float &perc, const Color &a, const Color &b );

extern ConVar sedit_draw_datatypes;
extern ConVar sedit_draw_shadows;
extern ConVar sedit_draw_nodes;
extern ConVar sedit_draw_jacks;
extern ConVar sedit_draw_bridges;

extern ConVar sedit_2dprev_Enable;
extern ConVar sedit_prev_HOMFun;
extern ConVar editor_movespeed;
extern ConVar editor_zoomspeed;
extern ConVar editor_pull_speed_mul;
extern ConVar editor_pull_speed_max;

extern ConVar sedit_mesh_size;
extern ConVar sedit_mesh_subdiv;

extern ConVar sedit_screenshot_zoom;
extern ConVar sedit_screenshot_quali;

const char *GetCanvasVersion_KeyName();
const char *GetCanvasVersion( int i );
const char *GetCanvasVersion_Current();
const int &GetCanvasVersion_Amt();
void UpgradeFromVersion( KeyValues *pCanvas, const char *oldVer );

const char *GetDumpVersion_KeyName();
const char *GetDumpVersion( int i );
const char *GetDumpVersion_Current();
const int &GetDumpVersion_Amt();

const char *GetEditorTitle();

enum HLSLNodeTypes
{
	HLSLNODE_INVALID = 0,

	HLSLNODE_VS_IN,
	HLSLNODE_VS_OUT,
	HLSLNODE_PS_IN,
	HLSLNODE_PS_OUT,

	HLSLNODE_CONSTANT_LOCAL,
	HLSLNODE_UNUSED_0,

	// sync with HLSLEnvironmentConstants !!
	HLSLNODE_CONSTANT_VIEW_ORIGIN,
	HLSLNODE_CONSTANT_VIEW_FORWARD,
	HLSLNODE_CONSTANT_VIEW_RIGHT,
	HLSLNODE_CONSTANT_VIEW_UP,
	HLSLNODE_CONSTANT_VIEW_WORLDSPACE_DEPTH,
	// till here
	HLSLNODE_CONSTANT_TIME,

	HLSLNODE_MATRIX_MODELVIEWPROJ,
	HLSLNODE_MATRIX_VIEWPROJ,
	HLSLNODE_MATRIX_MODEL,
	HLSLNODE_MATRIX_VIEWMODEL,

	HLSLNODE_MATH_ADD,
	HLSLNODE_MATH_SUBTRACT,
	HLSLNODE_MATH_MULTIPLY,
	HLSLNODE_MATH_DIVIDE,
	HLSLNODE_MATH_LERP,
	HLSLNODE_MATH_FRAC,

	HLSLNODE_MATH_DOT,
	HLSLNODE_MATH_LENGTH,
	HLSLNODE_MATH_NORMALIZE,
	HLSLNODE_MATH_APPEND,
	HLSLNODE_MATH_SWIZZLE,
	HLSLNODE_UNUSED_1,

	HLSLNODE_TEXTURE_SAMPLER,

	HLSLNODE_CONTROLFLOW_LOOP,
	HLSLNODE_CONTROLFLOW_COMBO,

	HLSLNODE_UTILITY_DECLARE,
	HLSLNODE_UTILITY_ASSIGN,

	HLSLNODE_MATH_SMOOTHSTEP,
	HLSLNODE_MATH_STEP,
	HLSLNODE_MATH_CLAMP,
	HLSLNODE_MATH_MIN,
	HLSLNODE_MATH_MAX,
	HLSLNODE_MATH_SIN,
	HLSLNODE_MATH_COS,
	HLSLNODE_MATH_TAN,
	HLSLNODE_MATH_ASIN,
	HLSLNODE_MATH_ACOS,
	HLSLNODE_MATH_ATAN,
	HLSLNODE_MATH_SINH,
	HLSLNODE_MATH_COSH,
	HLSLNODE_MATH_TANH,
	HLSLNODE_MATH_ATAN2,

	HLSLNODE_MATH_POW,
	HLSLNODE_MATH_ROUND,
	HLSLNODE_MATH_FLOOR,
	HLSLNODE_MATH_CEIL,
	HLSLNODE_MATH_ABS,
	HLSLNODE_MATH_FMOD,

	HLSLNODE_TEXTURE_TRANSFORM,

	HLSLNODE_MATH_DEGREES,
	HLSLNODE_MATH_RADIANS,

	HLSLNODE_VECTOR_REFLECT,
	HLSLNODE_VECTOR_REFRACT,

	HLSLNODE_MATH_SATURATE,
	HLSLNODE_MATH_INVERT,
	HLSLNODE_MATH_SIGN,
	HLSLNODE_MATH_SQRT,

	HLSLNODE_VECTOR_CROSS,

	HLSLNODE_CONSTANT_FB_PIXELSIZE,

	HLSLNODE_STUDIO_VERTEXLIGHTING,
	HLSLNODE_STUDIO_PIXELSHADER_LIGHTING,
	HLSLNODE_STUDIO_SKINNING,
	HLSLNODE_STUDIO_MORPH,

	HLSLNODE_CONSTANT_FOGPARAMS,
	HLSLNODE_UTILITY_FOG,
	HLSLNODE_UTILITY_FINAL,

	HLSLNODE_MATRIX_COMPOSE,

	HLSLNODE_STUDIO_PIXELSHADER_SPECLIGHTING,

	HLSLNODE_MATRIX_FLASHLIGHT,
	HLSLNODE_UTILITY_FLASHLIGHT,

	HLSLNODE_CONSTANT_CALLBACK,
	HLSLNODE_CONSTANT_VP_MUTABLE,
	HLSLNODE_CONSTANT_VP_STATIC,
	HLSLNODE_CONSTANT_LIGHTSCALE,
	HLSLNODE_CONSTANT_RANDOM,

	HLSLNODE_MATH_LOG,
	HLSLNODE_MATH_LOG2,
	HLSLNODE_MATH_LOG10,

	HLSLNODE_MATH_EXP,
	HLSLNODE_MATH_EXP2,

	HLSLNODE_CONSTANT_BUMPBASIS,

	HLSLNODE_MATH_DDX,
	HLSLNODE_MATH_DDY,

	HLSLNODE_MATRIX_CUSTOM,

	HLSLNODE_CONSTANT_ARRAY,

	HLSLNODE_CONTROLFLOW_CONDITION,
	HLSLNODE_CONTROLFLOW_BREAK,

	HLSLNODE_TEXTURE_SAMPLEROBJECT,

	HLSLNODE_TEXTURE_PARALLAX,
	HLSLNODE_TEXTURE_PARALLAX_SHADOW,

	HLSLNODE_CONSTANT_FLASHLIGHTPOS,

	HLSLNODE_CONTROLFLOW_CLIP,

	HLSLNODE_UTILITY_CUSTOMCODE,

	HLSLNODE_VECTOR_DISTANCE,

	HLSLNODE_STUDIO_VCOMPRESSION,


	/// UNRELATED
	HLSLNODE_OTHER_ = 1000,
	HLSLNODE_OTHER_COMMENT,


	/// POST PROCESSING
	HLSLNODE_POSTPROCESSING_ = 2000,
	HLSLNODE_POSTPROCESSING_INPUT,
	HLSLNODE_POSTPROCESSING_OUTPUT,

	HLSLNODE_POSTPROCESSING_CLEAR_BUFFERS,
	HLSLNODE_POSTPROCESSING_DRAW_MATERIAL,
	HLSLNODE_POSTPROCESSING_UPDATE_FB,

	HLSLNODE_POSTPROCESSING_PUSH_VP,
	HLSLNODE_POSTPROCESSING_POP_VP,
	HLSLNODE_POSTPROCESSING_SET_RT,

	HLSLNODE_POSTPROCESSING_CREATE_RT,
	HLSLNODE_POSTPROCESSING_CREATE_MAT,
	HLSLNODE_POSTPROCESSING_COPY_RT,

	HLSLNODE_POSTPROCESSING_RENDER_VIEW,

	HLSLNODE_,
};

enum HLSLVariableTypes
{
	HLSLVAR_FLOAT1 = ( 1 << 0 ),
	HLSLVAR_FLOAT2 = ( 1 << 1 ),
	HLSLVAR_FLOAT3 = ( 1 << 2 ),
	HLSLVAR_FLOAT4 = ( 1 << 3 ),

	HLSLVAR_MATRIX3X3 = ( 1 << 4 ),
	HLSLVAR_MATRIX4X3 = ( 1 << 5 ),
	HLSLVAR_MATRIX4X4 = ( 1 << 6 ),

	HLSLVAR_SAMPLER = ( 1 << 7 ),

	HLSLVAR_PP_MASTER = ( 1 << 16 ),
	HLSLVAR_PP_MATERIAL = ( 1 << 17 ),
	HLSLVAR_PP_RT = ( 1 << 18 ),

	HLSLVAR_COUNT = 19,

	// KEEP IN SYNC:
	// NUM_CBOX_VARTYPES
	// NUM_CBOX_VARTYPES_CFULL
	// GetTypeFlagFromEnum
	// VarTypeName
};

#define VSTOPS_NUM_TEXCOORDS 8
#define VSTOPS_NUM_COLORS 2

#define NUM_CBOX_VARTYPES 7
#define NUM_CBOX_VARTYPES_CFULL 8

enum HLSLJackVarCluster
{
	HLSLJACKFLAGS_F1,
	HLSLJACKFLAGS_F2,
	HLSLJACKFLAGS_F3,
	HLSLJACKFLAGS_F4,

	HLSLJACKFLAGS_F1_TO_F4,

	HLSLJACKFLAGS_VECTOR,
	HLSLJACKFLAGS_UV,
	HLSLJACKFLAGS_MATRIX,

	HLSLJACKFLAGS_ALL,
	HLSLJACKFLAGS_,
};

enum HLSLJackColorCode
{
	HLSLJACK_COLOR_NORM = 0,
	HLSLJACK_COLOR_RGB,
	HLSLJACK_COLOR_R,
	HLSLJACK_COLOR_G,
	HLSLJACK_COLOR_B,
	HLSLJACK_COLOR_A,
	HLSLJACK_COLOR_PPMASTER,
};

enum HLSLHierachyType
{
	HLSLHIERACHY_PENDING = -2, // for temp hierachy when sorting everthing

	HLSLHIERACHY_MULTIPLE = -1,
	HLSLHIERACHY_NONE = 0,
	HLSLHIERACHY_PS = ( 1 << 0 ),
	HLSLHIERACHY_VS = ( 1 << 1 ),
	//HLSLHIERACHY_TEMPLATE_PS = ( 1 << 2 ),
	//HLSLHIERACHY_TEMPLATE_VS = ( 1 << 3 ),
	HLSLHIERACHY_POST_PROCESS = ( 1 << 2 ),
};

enum NodeErrorLevel_t
{
	ERRORLEVEL_NONE = 0,
	ERRORLEVEL_UNDEFINED,
	ERRORLEVEL_FAIL,
};

enum RenderSystem_t
{
	RENDERMODE_SOFTWARE = 0,
	RENDERMODE_SOFTWARE_POSTPROCESS,
	RENDERMODE_HARDWARE,
	RENDERMODE_HARDWARE_POSTPROCESS,
	RENDERMODE_,
};

enum PreviewMode_t
{
	PVMODE_FINAL_COMBINED = 0,
	PVMODE_PER_NODE_CACHED,
};

enum
{
	LSCALE_LINEAR = 0,
	LSCALE_LIGHTMAP,
	LSCALE_ENVMAP,
	LSCALE_GAMMA,
	LSCALE_LIGHTMAP_RGB,

	LSCALE_MAX = ( LSCALE_GAMMA + 1 ),
};
enum
{
	ITERATORCOND_SMALLER = 0,
	ITERATORCOND_GREATER,
	ITERATORCOND_SEQUAL,
	ITERATORCOND_GEQUAL,
	ITERATORCOND_EQUAL,

	ITERATORCOND_,
};

struct customMatrix_t
{
	char *szdataTypeName;
	char *szIdentifierName;
	char *szCanvasName;
	int iRegSize;
	int iHLSLVarFlag;
};
const customMatrix_t *GetCMatrixInfo( int ID );

const char *GetConditionCodeString( int idx );
const char *GetCodeTonemapMacro( int idx );
const char *GetLightscaleCodeString( int idx );
const char *GetLightscaleGameString( int idx );

struct sampinfo_t
{
	const char *sz_visualName;
	const char *sz_codeName;
	int i_componentSize;
	bool b_derivativeInput;
	const char *sz_jackname;
};
enum
{
	TEXSAMP_AUTO = 0,

	TEXSAMP_1D,
	TEXSAMP_1D_DELTA,
	TEXSAMP_1D_BIAS,
	TEXSAMP_1D_GRAD,
	TEXSAMP_1D_LOD,
	TEXSAMP_1D_PROJ,

	TEXSAMP_2D,
	TEXSAMP_2D_DELTA,
	TEXSAMP_2D_BIAS,
	TEXSAMP_2D_GRAD,
	TEXSAMP_2D_LOD,
	TEXSAMP_2D_PROJ,

	TEXSAMP_3D,
	TEXSAMP_3D_DELTA,
	TEXSAMP_3D_BIAS,
	TEXSAMP_3D_GRAD,
	TEXSAMP_3D_LOD,
	TEXSAMP_3D_PROJ,

	TEXSAMP_CUBE,
	TEXSAMP_CUBE_DELTA,
	TEXSAMP_CUBE_BIAS,
	TEXSAMP_CUBE_GRAD,
	TEXSAMP_CUBE_LOD,
	TEXSAMP_CUBE_PROJ,

	TEXSAMP_MAX,
};

const char *GetSamplerData_VisualName( const int &idx );
const char *GetSamplerData_CodeName( const int &idx );
const int GetSampleData_ComponentSize( const int &idx );
const bool GetSamplerData_UseDerivative( const int &idx );
const char *GetSamplerData_JackName( const int &idx );

struct DynRenderHelper
{
	Vector2D Orig;
	float Size;
	float Rotation;
};

extern void PaintDynamicRotationStructure(DynRenderHelper info);

struct WriteContext_FXC;

#include "IVProcShader.h"
#include "ISEdit_ModelRender.h"
#include "PPEHelper.h"

float GaussianWeight_1D( const int &iSample_pos, const int &iKernel_size, const float &flBias = 0 );
float GaussianWeight_2D( const int &iPos_X, const int &iPos_Y, const int &iSize_X, const int &iSize_Y, const float &flBias = 0 );

void CreateDirectoryStrings();
void OverrideDirectories( const char *pszWorkingDirectory, const char *pszCompilePath, const char *pszLocalCompilePath,
		const char *pszGamePath, const char *pszCanvas, const char *pszShaderSource, const char *pszDumps,
		const char *pszUserFunctions, const char *pszEditorRoot );
void CreateDirectories();
void DestroyDirectoryStrings();
void PreviewCleanup();
#ifdef SHADER_EDITOR_DLL_SWARM
void InitSwarmShaders();
#endif

const char *GetWorkingDirectory();
const char *GetAccountName();
const char *GetGamePath();
const char *GetBinaryPath();
const char *GetBinaryPath_Local();
const char *GetCanvasDirectory();
const char *GetShaderSourceDirectory();
const char *GetDumpDirectory();
const char *GetUserFunctionDirectory();
const char *GetEditorRootDirectory();
#ifdef SHADER_EDITOR_DLL_SWARM
const char *GetSwarmShaderDirectory();
#endif

const bool IsInEditMode();

void ComposeShaderName( GenericShaderData *data, bool bPS, bool bExtension, char *out, int maxbuf, bool bPosOverride = false ); // fxc
void ComposeShaderPath( GenericShaderData *data, bool bPS, bool bExtension, char *out, int maxbuf, bool bPosOverride = false );
void ComposeShaderName_Compiled( GenericShaderData *data, bool bPS, bool bExtension, char *out, int maxbuf, bool bPosOverride = false ); // vcs
void ComposeShaderPath_Compiled( GenericShaderData *data, bool bPS, bool bExtension, char *out, int maxbuf, bool bPosOverride = false );
void ComposeShaderPath_CompiledEngine( GenericShaderData *data, bool bPS, bool bExtension, char *out, int maxbuf, bool bPosOverride = false );

struct __outputsetup;
void BuildUserFunctionName( const char *pFunctionName, const CUtlVector< __funcParamSetup* > &sharedOutputs, char *pszDest, int buflen );
void BuildUserFunctionName( const char *pFunctionName, const CUtlVector< __funcParamSetup* > &input, const CUtlVector< __funcParamSetup* > &output,
							char *pszDest, int buflen );
void BuildDefaultFunctionName( char *pszDest, int buflen, CBaseNode *pNode );

const char *GetTextureTypeName( int idx );
const char *GetVarTypeName( int num );
const char *GetVarTypeNameCode( int num );
const char *GetResourceTypeName( int num );
const char *GetSemanticTypeName( int num );
const char *GetSemanticType( int num );

int GetVarTypeRowsRequired( int num );
int GetVarTypeFlag( int num );
int GetVarFlagsVarValue( int flag );
const char *GetVarCodeNameFromFlag( int flag );
int GetVarFlagsRowsRequired( int flag );

extern inline void AutoCopyFloats( const void *src, void *dst, const int amt );
extern inline void AutoCopyStringPtr( const char *src, char **dst );

//extern inline int GetChannelNumFromChar( const char *c );
//extern inline char GetCharFromChannelNum( const int i );

extern inline int GetSlotsFromTypeFlag( int flag );
extern inline int GetTypeFlagFromEnum( int i );

extern inline void UpdateSimpleObjectBounds( Vector2D &pos, Vector2D &size, Vector4D &bounds );
extern inline bool ShouldSimpleDrawObject( vgui::Panel *parent, CNodeView *coordSystem, Vector4D &bounds );

extern inline void ClipToScreenBounds( int &_x, int &_y, int &_sx, int &_sy );

template <class T>
void SaveDeleteVector( CUtlVector<T> &m_hItems )
{
	CUtlVector< T > m_hTmp;
	m_hTmp.AddVectorToTail( m_hItems );
	m_hItems.Purge();
	m_hTmp.PurgeAndDeleteElements();
}
void DestroySolverStack( CUtlVector< CHLSL_SolverBase* > &m_hSolvers );
void DestroyVariablesInSolverStack( CUtlVector< CHLSL_SolverBase* > &m_hSolvers );

void ListVariables( CUtlVector< CHLSL_SolverBase* > &m_hSolvers, CUtlVector< CHLSL_Var* > &m_hVars );
void InitializeVariableNames( CUtlVector< CHLSL_SolverBase* > &m_hSolvers, WriteContext_FXC &context );
void ResetVariables( CUtlVector< CHLSL_SolverBase* > &m_hSolvers );
void ClearVariableInfo( CUtlVector< CHLSL_SolverBase* > &m_hSolvers );
void ResetVariableIndices( CUtlVector< CHLSL_SolverBase* > &m_hSolvers );

void ClearSolverInfo( CUtlVector< CHLSL_SolverBase* > &m_hSolvers );

void CopySolvers( const CUtlVector< CBaseNode* > &m_hSolvedNodes,
					CUtlVector< CHLSL_SolverBase* > &m_hOutput );
void CopySolvers( const CUtlVector< CHLSL_SolverBase* > &m_hInput,
					CUtlVector< CHLSL_SolverBase* > &m_hOutput );

void ClearIdentifiers( GenericShaderData &data );
void SetupForRendering( CUtlVector< CHLSL_SolverBase* > &hStack );
void AllocateIdentifiers( IdentifierLists_t &Identifiers, CUtlVector< CHLSL_SolverBase* > &hStack );
void UniquifyIdentifiers( bool bPS, IdentifierLists_t &Identifiers );
void CalcNumDynamicCombos( IdentifierLists_t &Identifiers, bool bPreview );

int StringTabsWorth( const char *text );
void CleanupString( const char *pSrc, char *pOut, int maxLength, bool AllowSpaces = false, bool AllowUnderscore = true, bool AllowNumbers = true, bool AllowNumberInFirstPlace = false );

const char *GetComboNameByID( const int &ID );
const int GetComboIDByName( const char *name );
void UpdateComboDataByString( SimpleCombo *pCombo, const char *szName, bool bTotalInit = true );
void UpdateComboDataByID( SimpleCombo *pCombo, const int &ID, bool bTotalInit = true );
SimpleCombo *AllocateComboDataByID( const int &ID );
const bool IsComboAvailableInPreviewMode( SimpleCombo *c );
void RemoveAllNonPreviewCombos( CUtlVector< SimpleCombo* >&list );

typedef unsigned int HCURSTACKIDX;

#include "materialsystem/itexture.h"
void BuildFormatFlags( BasicShaderCfg_t &data, CUtlVector< CHLSL_SolverBase* > &hVertexshaderStack, CUtlVector< CHLSL_SolverBase* > &hPixelhaderStack );
struct econstdata
{
	char *varname;
	char *jackname;
	int varflag;
	char *nodename;
	int nodetype;
	void (*PreviewSolve)(float *_fl4);
};
econstdata *EConstant_GetData( int i );


ITexture *GetFBTex();
ITexture *GetFBTexSmall();
ITexture *GetPingPongRT( int num );
ITexture *GetFBCopyTex();
ITexture *GetSkymaskTex();

const bool IsSkymaskAvailable();
const SEDIT_SKYMASK_MODE GetSkymaskMode();

void AllocRTs( int size );
void AllocSkymask( SEDIT_SKYMASK_MODE iSkymaskMode );
//void DeallocPingPongRTs();
const int &GetRTSize();
const int &GetRTPuzzleSize();

void ResetTileInfo();
void CalcPuzzleSize( int numSolvers );

const int &GetCurrentPuzzleIndex();
void OnPuzzleDrawn();
void GetUVsForPuzzle( const int &num, Vector2D &min, Vector2D &max, bool bAddClamp = false );
void GetCurPuzzleView( int &x, int &y, int &w, int &t );

void ReloadGameShaders( GenericShaderData *data, char **pszMaterialList = NULL, int iNumMaterials = 0 );
void DoMaterialReload();
void QuickRefreshEditorShader();

struct SETUP_HLSL_VS_Input
{
	SETUP_HLSL_VS_Input()
	{
		iTexcoords_num = 1;
		iVertexcolor_num = 0;
		bFlexDelta_enabled = false;
		bNormal_enabled = false;
		bTangentspace_enabled = false;
		bBlendweights_enabled = false;
		bNormal_enabled_compression = false;
		bTangentspace_enabled_skinning = false;
		for ( int i = 0; i < 3; i++ )
			iDataTypeFlag_TexCoords[i] = HLSLVAR_FLOAT2;
		for ( int i = 0; i < 2; i++ )
			iDataTypeFlag_Color[i] = HLSLVAR_FLOAT4;
	};
	int iTexcoords_num;
	int iVertexcolor_num;

	bool bFlexDelta_enabled;
	bool bNormal_enabled;
	bool bTangentspace_enabled;
	bool bBlendweights_enabled;

	bool bNormal_enabled_compression;
	bool bTangentspace_enabled_skinning;

	int iDataTypeFlag_TexCoords[3];
	int iDataTypeFlag_Color[2];
};
struct SETUP_HLSL_VS_Output_PS_Input
{
	SETUP_HLSL_VS_Output_PS_Input()
	{
		iTexcoords_num = 1;
		iVertexcolor_num = 0;
		for ( int i = 0; i < VSTOPS_NUM_TEXCOORDS; i++ )
			iTexCoordFlag[ i ] = HLSLVAR_FLOAT2;
		for ( int i = 0; i < VSTOPS_NUM_COLORS; i++ )
			iColorFlag[ i ] = HLSLVAR_FLOAT4;
	};
	int iTexcoords_num;
	int iVertexcolor_num;

	int iTexCoordFlag[ VSTOPS_NUM_TEXCOORDS ];
	int iColorFlag[ VSTOPS_NUM_COLORS ];
};
struct SETUP_VSOUTPSIN_CustomNames
{
	SETUP_VSOUTPSIN_CustomNames()
	{
		for ( int i = 0; i < VSTOPS_NUM_TEXCOORDS; i++ )
			names_texcoords[i][0] = '\0';
		for ( int i = 0; i < VSTOPS_NUM_COLORS; i++ )
			names_colors[i][0] = '\0';
	};

	char names_texcoords[VSTOPS_NUM_TEXCOORDS][20];
	char names_colors[VSTOPS_NUM_COLORS][20];
};
struct SETUP_HLSL_PS_Output
{
	SETUP_HLSL_PS_Output()
	{
		iNumRTs = 1;
		bDepth = false;
	};
	int iNumRTs;
	bool bDepth;
};


/*
//// NOTES ////
// tangentspace
// tangent S uvdir: 0,0 - 1,0
// tangent T uvdir: 0,1 - 0,0 // flipped
///////////////
*/
// per vertex
struct Solver_VS_Input
{
	//Solver_VS_Input();

	float vecPos[3];
	float vecNormal[3];
	float vecTangentS[4];
	float vecTangentT[3];
	float vecFlexDelta[3];

	float boneWeights[4];
	float boneIndices[4];

	float texCoord_[3][2];
	float color_[2][4];
};
struct Solver_VSOut_PSIn
{
	//Solver_VSOut_PSIn();

	// VS only
	float vecProjPos[4];

	float texCoord_[8][4];
	float color_[2][4];
};
struct Solver_PS_Output
{
	//Solver_PS_Output();

	float color_[4][4];
	float depth;
};
struct SolverResources
{
	SolverResources( int HierachyType );

	const int GetHierachyType();
	bool IsVertexShader();

	Solver_VSOut_PSIn vsout_psin;
	union
	{
		Solver_VS_Input vs_in;
		Solver_PS_Output ps_out;
	};

	VMatrix mx_Model;
	VMatrix mx_View;
	VMatrix mx_Proj;
private:
	int m_iHierachyType;
};

struct Preview2DContext
{
public:
	Preview2DContext();

	IMatRenderContext *pRenderContext;

	IMaterial *pMat_PsIn;
	IMaterialVar *pMat_PsInOP;
};

struct GenericShaderData
{
public:
	GenericShaderData();
	~GenericShaderData();
	GenericShaderData( const GenericShaderData &o );

	//
	char *name;
	bool IsPreview();
	//bool bPreview;
	HCURSTACKIDX iStackIdx;

	SETUP_HLSL_VS_Input VS_IN;
	SETUP_HLSL_VS_Output_PS_Input VS_to_PS;
	SETUP_HLSL_PS_Output PS_OUT;
	//

	BasicShaderCfg_t *shader;
	IdentifierLists_t *pUNDEF_Identifiers;
};

struct GenericPPEData
{
public:
	GenericPPEData();
	~GenericPPEData();

	bool bDoAutoUpdateFBCopy;
};

struct WriteContext_FXC
{
	WriteContext_FXC();

	CUtlBufferEditor buf_combos;
	CUtlBufferEditor buf_samplers;
	CUtlBufferEditor buf_constants;
	CUtlBufferEditor buf_arrays;
	CUtlBufferEditor buf_functions_globals;
	CUtlBufferEditor buf_functions_bodies;
	CUtlBufferEditor buf_semantics_In;
	CUtlBufferEditor buf_semantics_Out;
	CUtlBufferEditor buf_code;

	bool bPreview;

	CUtlVector< CHLSL_SolverBase* > *m_hActive_Solvers;
	IdentifierLists_t *m_pActive_Identifiers;

private:
	WriteContext_FXC( const WriteContext_FXC &o );
};

struct RunCodeContext
{
public:
	RunCodeContext( bool bPreview = false, bool bScene = false );

	IMatRenderContext *pRenderContext;
	const bool &IsUsingPreview() const;
	const bool &IsSceneMode() const;

private:
	bool bPreviewMode;
	bool bSceneMode;
};

struct BridgeRestoreInfo
{
	BridgeRestoreInfo()
	{
		idx = -1;
		resType = -1;
		iMode = -1;
	};
	CJack *pJackOther;

	int idx;
	char jackSelfName[80];
	int resType;

	int iMode;
};

class ISolverRequester
{
public:

	virtual void OnSolverUpdated( CNodeView *pCaller, GenericShaderData *pData,
		bool VSReady, bool PSReady ) = 0;
	virtual void OnSolverErrored() = 0;
};

enum FodPathContexts
{
	FODPC_VTF = 0,
	FODPC_MDL,
	FODPC_CANVAS,
	FODPC_DUMPFILE,
	FODPC_USERFUNCTION,
	FODPC_VMT,

	FODPC_,
};
const char *GetFODPathContext( int type );

struct LimitReport_t
{
	char *sz_name; // should always point to a string literal
	int i_cur;
	int i_max;
};
enum
{
	LERROR_PS_SAMPLER,
	LERROR_PS_CONSTS,

	LERROR_VS_SAMPLER,
	LERROR_VS_CONSTS,

	LERROR_VMT_PARAMS_STATIC,
	LERROR_VMT_PARAMS_MUTABLE,

	LERROR_GENERIC_DEMOTEXTURE_MISMTACH,
	LERROR_,
};
#define LERROR__FIRST LERROR_PS_SAMPLER
#define LERROR__LAST LERROR_VMT_PARAMS_MUTABLE

const LimitReport_t &GetLimitReporterDefault( int i );
bool ReportErrors( GenericShaderData *data, CUtlVector< LimitReport_t* > &hErrorList );

#define RT_NAME_MAX 64

struct RTDef
{
public:
	RTDef();
	RTDef( const char *pszName,
		ImageFormat format, RenderTargetSizeMode_t sizemode, MaterialRenderTargetDepth_t depthmode,
		int x, int y, unsigned int rtflags );
	~RTDef();

	bool IsValid();

	void Allocate();
	void Deallocate();
	void Modify( const char *pszName = NULL,
		ImageFormat *format = NULL, RenderTargetSizeMode_t *sizemode = NULL, MaterialRenderTargetDepth_t *depthmode = NULL,
		int *x = NULL, int *y = NULL, unsigned int *rtflags = NULL );
	void Modify( RTDef &src );

	bool IsDirty();
	void MakeDirty();
	bool IsFlaggedForDeletion();
	void FlagForDeletion();
	void Revalidate();

	const char *GetName() const;
	ImageFormat GetImgFormat();
	RenderTargetSizeMode_t GetSizeMode();
	MaterialRenderTargetDepth_t GetDepthMode();
	unsigned int GetFlags();
	int GetSizeX();
	int GetSizeY();

private:
	//RTDef( const RTDef &o );

	char szRTName[RT_NAME_MAX];
	ImageFormat Format;
	MaterialRenderTargetDepth_t DepthMode;
	RenderTargetSizeMode_t SizeMode;
	unsigned int iRTFlags;
	int x, y;

	bool bDirty;
	bool bFlaggedForDeletion;

	ITexture *ref;
};

#include "tier1/smartptr.h"

#define MAX_ZOOM_FONTS 85

#define CRUCIALNODE_MIN_SIZE_X 55

#define CRUCIALNODE_CANVAS_BASE_X 320
#define CRUCIALNODE_CANVAS_BASE_Y 160
#define CRUCIALNODE_CANVAS_OFFSET_Y 30

#define PREVIEW_SIZE_NODE 80

#define SHEET_SLIDER_THUMB_SIZE 12

typedef HNODE HAUTOSOLVER;

void ForceDeleteMaterial( IMaterial **pMat );
void ForceDeleteTexture( ITexture **pTex );
IMaterialVar *FindMaterialVar( IMaterial *pMat, const char *varName );
bool SetMaterialVar_String( IMaterial *pMat, const char *varName, const char *value );
bool SetMaterialVar_Vec( IMaterial *pMat, const char *varName, const float a, const float b, const float c, const float d );
bool SetMaterialVar_Vec( IMaterial *pMat, const char *varName, const float a, const float b, const float c );
bool SetMaterialVar_Vec( IMaterial *pMat, const char *varName, const float a, const float b );
bool SetMaterialVar_Vec( IMaterial *pMat, const char *varName, const float a );
bool SetMaterialVar_Int( IMaterial *pMat, const char *varName, const int a );

void GeneralFramebufferUpdate( IMatRenderContext *pRenderContext );

#include "filesystem.h"
#include "utlbuffer.h"
#include "EditorInit.h"
#include "gShaderDumps.h"
#include "gRendertargetConfig.h"
#include "gPostProcessingCache.h"
#include "cKVPacker.h"

#include "vgui_controls/Controls.h"
#include <vgui_controls/propertydialog.h>
#include <vgui_controls/propertysheet.h>
#include <vgui_controls/propertypage.h>
#include <vgui_controls/textentry.h>
#include <vgui_controls/checkbutton.h>
#include <vgui_controls/slider.h>
#include <vgui_controls/combobox.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui_controls/fileopendialog.h>
#include <vgui_controls/radiobutton.h>
#include <vgui_controls/promptsimple.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui/ILocalize.h>

#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"

#include "cHLSL_Image.h"
#include "cHLSL_Mesh.h"
#include "cHLSL_Variable.h"
#include "hlsl_solver.h"

#include "vBridge.h"
#include "vJack.h"
#include "vBaseNode.h"

#include "vBaseContainer.h"
#include "vNode_Matrices.h"
#include "vNode_Add.h"
#include "vNode_Subtract.h"
#include "vNode_Multiply.h"
#include "vNode_Divide.h"
#include "vNode_Cross.h"
#include "vNode_Swizzle.h"
#include "vNode_Dot.h"
#include "vNode_VecAppend.h"
#include "vNode_VectorRef.h"
#include "vNode_BaseEvaluate.h"
#include "vNode_Lerp.h"
#include "vNode_BaseRange.h"
#include "vNode_Constant.h"
#include "vNode_EnvC.h"
#include "vNode_Utility.h"
#include "vNode_MCompose.h"
#include "vNode_Callback.h"
#include "vNode_VmtParam.h"
#include "vNode_Array.h"
#include "vNode_Break.h"
#include "vNode_Custom.h"

#include "vNode_TexSample.h"
#include "vNode_TexTransform.h"
#include "vNode_Sampler.h"
#include "vNode_Parallax.h"

#include "vNode_Loop.h"
#include "vNode_Combo.h"
#include "vNode_Condition.h"

#include "vNode_Std_VLight.h"
#include "vNode_Std_PLight.h"
#include "vNode_Std_Skinning.h"
#include "vNode_VCompression.h"

#include "vNode_Fog.h"
#include "vNode_Final.h"
#include "vNode_Flashlight.h"
#include "vNode_Lightscale.h"

#include "vNode_VsInput.h"
#include "vNode_VsOutput.h"
#include "vNode_PsInput.h"
#include "vNode_PsOutput.h"



#include "vNode_PP_Base.h"
#include "vNode_PP_RT.h"
#include "vNode_PP_Mat.h"
#include "vNode_PP_Operations.h"
#include "vNode_PP_DrawMat.h"
#include "vNode_PP_RenderView.h"



#include "vNode_Comment.h"



#include "vBaseDiag.h"
#include "vDialogNew.h"
#include "vDialogConfig.h"
#include "vDialogPPEConfig.h"
#include "vDialogSPrecache.h"
#include "vDialogEditorConfig.h"
#include "vDialogRTList.h"
#include "vDialogPPEffectPrecache.h"
#include "vDialogRecompileAll.h"

#include "vNodeViewError.h"
#include "vNodeView.h"
#include "vEditorFlowGraphPage.h"
#include "vEditorFlowGraphSheet.h"
#include "vEditorRoot.h"
#include "vRenderPanel.h"
#include "vPreview.h"



enum
{
	TCMD_COMMON_NONE = 0,
	TCMD_COMMON_EXIT,
};

struct __threadcmds_common
{
	__threadcmds_common(){};
	int _cmd;
};
struct __threadcmds_CompileCommand
{
	__threadcmds_CompileCommand( HAUTOSOLVER Caller )
	{
		__vs = NULL;
		__ps = NULL;
		__undef = NULL;
		__data = NULL;
		_Requester = Caller;
	//	bHaltOnError = false;

		AllocCheck_Alloc();
	};
	~__threadcmds_CompileCommand()
	{
		delete __vs;
		delete __ps;
		delete __undef;
		delete __data;

		AllocCheck_Free();
	};
	void *__vs;
	void *__ps;
	void *__undef;

	GenericShaderData *__data;

	HAUTOSOLVER _Requester;

	//bool bHaltOnError;
};
struct __threadcmds_CompileCallback
{
	__threadcmds_CompileCallback( HAUTOSOLVER Target )
	{
		_Target = Target;
		_ExitCode = 0;
		_data = NULL;
		_activeFlags = 0;

		AllocCheck_Alloc();
	};

	~__threadcmds_CompileCallback()
	{
		delete _data;

		AllocCheck_Free();
	};

	HAUTOSOLVER _Target;
	int _ExitCode;
	int _activeFlags;

	GenericShaderData *_data;
};

extern CEditorRoot *pEditorRoot;

#include "cSolverCallback.h"
//#include "custom/cSolveThread.h"
#include "cBaseThread.h"
#include "cCompileThread.h"
#include "cThreadManager.h"

#endif