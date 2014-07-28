#include "cbase.h"

#include <vgui/VGUI.h>
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include "ienginevgui.h"

#include "vgui_controls/Controls.h"
#include "editorcommon.h"


ConVar sedit_draw_datatypes( "shaderEditor_render_datatypes", "1" );
ConVar sedit_draw_shadows( "shaderEditor_render_shadows", "1" );
ConVar sedit_draw_nodes( "shaderEditor_render_nodes", "1" );
ConVar sedit_draw_jacks( "shaderEditor_render_jacks", "1" );
ConVar sedit_draw_bridges( "shaderEditor_render_bridges", "1" );

ConVar sedit_2dprev_Enable( "shaderEditor_2dpreview_enable_draw", "1" );
ConVar sedit_uv_adjust( "shaderEditor_2dpreview_adjustUV", "1" );

ConVar editor_movespeed( "shaderEditor_smooth_speed", "10" );
ConVar editor_zoomspeed( "shaderEditor_zoom_speed", "0.65f" );
ConVar editor_pull_speed_mul( "shaderEditor_pull_speed_multiply", "75" );
ConVar editor_pull_speed_max( "shaderEditor_pull_speed_max", "20" );

ConVar sedit_mesh_size( "shaderEditor_3dpreview_meshSize", "50" );
ConVar sedit_mesh_subdiv( "shaderEditor_3dpreview_meshSubdiv", "48" );
ConVar sedit_prev_HOMFun( "shaderEditor_3dpreview_HOMFun", "0" );

ConVar sedit_screenshot_zoom( "shaderEditor_screenshot_zoom", "2" );
ConVar sedit_screenshot_quali( "shaderEditor_screenshot_quality", "100" );

using namespace vgui;

#define BOUNDS_EXTRUDE 30

const char *GetEditorTitle()
{
	return "Source Shader Editor v0.5 (Build 0244)";
};

static const char *cversion_key = "cfmt_version";
static const char *cversions[] = {
	"canvasver#UNDEFINED",
	"canvasver#002",
	"canvasver#003",
};
static const int cversions_num = ARRAYSIZE( cversions );

static const char *dumpversion_key = "dump_version";
static const char *dumpversions[] = {
	"dumpver#000",
	"dumpver#001",
};
static const int dumpversions_num = ARRAYSIZE( dumpversions );

const char *GetCanvasVersion_KeyName()
{
	return cversion_key;
}
const char *GetCanvasVersion( int i )
{
	Assert( i >= 0 && i < cversions_num );
	return cversions[ i ];
}
const char *GetCanvasVersion_Current()
{
	return GetCanvasVersion( cversions_num - 1 );
}
const int &GetCanvasVersion_Amt()
{
	return cversions_num;
}

const char *GetDumpVersion_KeyName()
{
	return dumpversion_key;
}
const char *GetDumpVersion( int i )
{
	Assert( i >= 0 && i < dumpversions_num );
	return dumpversions[ i ];
}
const char *GetDumpVersion_Current()
{
	return GetDumpVersion( dumpversions_num - 1 );
}
const int &GetDumpVersion_Amt()
{
	return dumpversions_num;
}

void UpgradeFromVersion( KeyValues *pCanvas, const char *oldVer )
{
	bool bDoUpgrade = oldVer == NULL;

	for ( int i = 0; i < GetCanvasVersion_Amt(); i++ )
	{
		bDoUpgrade = bDoUpgrade || !Q_stricmp( oldVer, GetCanvasVersion( i ) );

		if ( !bDoUpgrade )
			continue;

		KeyValues *pNodes = pCanvas->FindKey( "nodes" );

		// upgrading to version i from version i - 1....
		switch ( i )
		{
			// from canvasver#002 to canvasver#003
		case 2:
			{
				if ( !pNodes )
					break;

				for ( int idx = 0;; idx++ )
				{
					char tmp[MAX_PATH];
					Q_snprintf( tmp, MAX_PATH, "Node_%03i", idx );
					KeyValues *pKVNode = pNodes->FindKey( tmp );
					idx++;

					if ( !pKVNode )
						break;

					if ( pKVNode->GetInt( "iType" ) != HLSLNODE_VS_IN )
						continue;

					int dtype_tex = pKVNode->GetInt( "dTFlag_Texcoords", 0 );
					int dtype_col = pKVNode->GetInt( "dTFlag_Color", 0 );

					if ( dtype_tex > 0 )
						for ( int u = 0; u < 3; u++ )
							pKVNode->SetInt( VarArgs( "dTFlag_Texcoords_%i", u ), dtype_tex );
					if ( dtype_col > 0 )
						for ( int u = 0; u < 2; u++ )
							pKVNode->SetInt( VarArgs( "dTFlag_Color_%i", u ), dtype_col );
				}
			}
			break;
		}
	}
}

Color Lerp( const float &perc, const Color &a, const Color &b )
{
	return Color( a[0] + (b[0] - a[0]) * perc,
		a[1] + (b[1] - a[1]) * perc,
		a[2] + (b[2] - a[2]) * perc,
		a[3] + (b[3] - a[3]) * perc );
}

void PaintDynamicRotationStructure(DynRenderHelper info)
{
	Vector AngDir;
	float rot = info.Rotation + 45.0f;

	QAngle Ang = QAngle(0,rot,0);
	AngleVectors(Ang,&AngDir);
	vgui::Vertex_t verts[4];
	float pxcenterx, pxcentery, px1x, px1y, px2x, px2y, pxoutx, pxouty;
	Vector Direction = Vector(AngDir.x,AngDir.y,0);
	Vector2D Dir, DirCrossed, Pos;
	Pos.x = info.Orig.x;
	Pos.y = info.Orig.y;

	VectorNormalize(Direction);
	Vector CrossHelper = vec3_origin;
	CrossProduct(Vector(0,0,1), Direction, CrossHelper);
	CrossHelper.z = 0.0f;
	VectorNormalize(CrossHelper);
	Direction *= info.Size;
	CrossHelper *= info.Size;
	Dir.x = Direction.x;
	Dir.y = Direction.y;
	DirCrossed.x = CrossHelper.x;
	DirCrossed.y = CrossHelper.y;

	pxoutx = Pos.x + Dir.x;
	pxouty = Pos.y + Dir.y;
	pxcenterx = Pos.x - Dir.x;
	pxcentery = Pos.y - Dir.y;

	px1x = Pos.x - DirCrossed.x;
	px1y = Pos.y - DirCrossed.y;
	px2x = Pos.x + DirCrossed.x;
	px2y = Pos.y + DirCrossed.y;

	float uv_minx, uv_maxx, uv_miny, uv_maxy;
	uv_minx = 0.0f;
	uv_maxx = 1.0f;
	uv_miny = 0.0f;
	uv_maxy = 1.0f;

	verts[0].m_TexCoord.Init( uv_minx, uv_miny );
	verts[0].m_Position.Init(pxcenterx, pxcentery);
	verts[1].m_TexCoord.Init( uv_maxx, uv_miny );
	verts[1].m_Position.Init(px1x, px1y);
	verts[2].m_TexCoord.Init( uv_maxx, uv_maxy );
	verts[2].m_Position.Init(pxoutx, pxouty);
	verts[3].m_TexCoord.Init( uv_minx, uv_maxy );
	verts[3].m_Position.Init(px2x, px2y);

	vgui::surface()->DrawTexturedPolygon( 4, verts );
}

inline void UpdateSimpleObjectBounds( Vector2D &pos, Vector2D &size, Vector4D &bounds )
{
	bounds.x = pos.x - BOUNDS_EXTRUDE;
	bounds.y = pos.y + BOUNDS_EXTRUDE;
	bounds.z = pos.x + size.x + BOUNDS_EXTRUDE;
	bounds.w = pos.y + size.y - BOUNDS_EXTRUDE;
}
inline bool __InBounds( Vector2D &p, Vector2D &smin, Vector2D &smax )
{
	if ( p.x >= smin.x && p.x <= smax.x &&
		p.y >= smin.y && p.y <= smax.y )
		return true;
	return false;
}
inline bool __CrossesBounds( float &p_min, float &p_max, float &b_min, float &b_max )
{
	if ( p_max < b_min )
		return false;
	if ( p_min > b_max )
		return false;
	return true;
}
inline bool ShouldSimpleDrawObject( vgui::Panel *parent, CNodeView *coordSystem, Vector4D &bounds )
{
	Vector2D bpoint_00( bounds.x, bounds.y );
	Vector2D bpoint_11( bounds.z, bounds.w );

	coordSystem->ToPanelSpace( bpoint_00 );
	coordSystem->ToPanelSpace( bpoint_11 );

	int sx,sy;
	parent->GetSize( sx, sy );
	Vector2D p_min( 0, 0 );
	Vector2D p_max( sx, sy );

	if ( !__CrossesBounds( bpoint_00.x, bpoint_11.x, p_min.x, p_max.x ) ||
		!__CrossesBounds( bpoint_00.y, bpoint_11.y, p_min.y, p_max.y ) )
		return false;

	return true;
}
void DestroySolverStack( CUtlVector< CHLSL_SolverBase* > &m_hSolvers )
{
	DestroyVariablesInSolverStack( m_hSolvers );
	SaveDeleteVector( m_hSolvers );
}
void DestroyVariablesInSolverStack( CUtlVector< CHLSL_SolverBase* > &m_hSolvers )
{
	CUtlVector< CHLSL_Var* > m_hVars;
	ListVariables( m_hSolvers, m_hVars );
	SaveDeleteVector( m_hVars );
}
void ResetVariables( CUtlVector< CHLSL_SolverBase* > &m_hSolvers )
{
	CUtlVector< CHLSL_Var* > m_hVars;
	ListVariables( m_hSolvers, m_hVars );
	for ( int i = 0; i < m_hVars.Count(); i++ )
	{
		CHLSL_Var *v = m_hVars[ i ];
		v->ResetVarInfo();
	}
	m_hVars.Purge();
}
void ListVariables( CUtlVector< CHLSL_SolverBase* > &m_hSolvers, CUtlVector< CHLSL_Var* > &m_hVars )
{
	m_hVars.Purge();
	for ( int a = 0; a < m_hSolvers.Count(); a++ )
	{
		CHLSL_SolverBase *pS = m_hSolvers[ a ];
		for ( int b = 0; b < pS->GetNumTargetVars(); b++ )
		{
			CHLSL_Var *pV = pS->GetTargetVar( b );
			if ( m_hVars.IsValidIndex( m_hVars.Find( pV ) ) )
				continue;
			m_hVars.AddToTail( pV );
		}
	}
}
void InitializeVariableNames( CUtlVector< CHLSL_SolverBase* > &m_hSolvers, WriteContext_FXC &context )
{
	context.m_hActive_Solvers = &m_hSolvers;
	CUtlVector< CHLSL_Var* > m_hVars;
	ListVariables( m_hSolvers, m_hVars );
	int _index = 0;

	for ( int i = 0; i < m_hSolvers.Count(); i++ )
		m_hSolvers[i]->Invoke_VarInit( context );

	char tmp[MAXTARGC];
	for ( int i = 0; i < m_hVars.Count(); i++ )
	{
		CHLSL_Var *var = m_hVars[ i ];
		if ( !var->GetName() || !var->HasCustomName() )
		{
			Q_snprintf( tmp, MAXTARGC, "_var%i", _index );
			var->SetName( tmp );
			_index++;
		}
	}

	for ( int i = 0; i < m_hSolvers.Count(); i++ )
		m_hSolvers[i]->Invoke_VarInit_PostStep();

	m_hVars.Purge();
}
void ClearSolverInfo( CUtlVector< CHLSL_SolverBase* > &m_hSolvers )
{
	for ( int i = 0; i < m_hSolvers.Count(); i++ )
	{
		CHLSL_SolverBase *s = m_hSolvers[ i ];
		s->ClearInfo();
	}
}
void ClearVariableInfo( CUtlVector< CHLSL_SolverBase* > &m_hSolvers )
{
	CUtlVector< CHLSL_Var* > m_hVars;
	ListVariables( m_hSolvers, m_hVars );
	for ( int i = 0; i < m_hVars.Count(); i++ )
	{
		CHLSL_Var *v = m_hVars[ i ];
		v->Cleanup();
	}
	m_hVars.Purge();
}
void ResetVariableIndices( CUtlVector< CHLSL_SolverBase* > &m_hSolvers )
{
	CUtlVector< CHLSL_Var* > m_hVars;
	ListVariables( m_hSolvers, m_hVars );
	for ( int i = 0; i < m_hVars.Count(); i++ )
	{
		CHLSL_Var *v = m_hVars[ i ];
		v->ResetMapIndex();
	}
	m_hVars.Purge();
}
void CopySolvers( const CUtlVector< CBaseNode* > &m_hSolvedNodes,
					CUtlVector< CHLSL_SolverBase* > &m_hOutput )
{
	CUtlVector< CBaseNode* > m_hMainList;
	CUtlVector< CHLSL_SolverBase* > m_hMainList_Solvers;
	m_hMainList.AddVectorToTail( m_hSolvedNodes );

	CUtlVector< CBaseContainerNode* > m_hContainerStack;
	for ( int a = 0; a < m_hMainList.Count(); a++ )
	{
		CBaseNode *pN = m_hMainList[ a ];
		CBaseContainerNode *pC = pN->GetAsContainer();

		// enter and exit containers
		if ( pC && pC->GetNumSolvers() >= 3 )
		{
			if ( m_hContainerStack.IsValidIndex( m_hContainerStack.Find( pC ) ) )
			{
				m_hContainerStack.FindAndRemove( pC );
				m_hMainList_Solvers.AddToTail( pC->GetSolver( 2 ) );
			}
			else
			{
				m_hContainerStack.AddToTail( pC );
				m_hMainList_Solvers.AddToTail( pC->GetSolver( 0 ) );
				m_hMainList_Solvers.AddToTail( pC->GetSolver( 1 ) );
			}
			continue;
		}

		for ( int b = 0; b < pN->GetNumSolvers(); b++ )
		{
			CHLSL_SolverBase *pS = pN->GetSolver( b );
			if ( m_hMainList_Solvers.IsValidIndex( m_hMainList_Solvers.Find( pS ) ) )
				continue;
			m_hMainList_Solvers.AddToTail( pS );
		}
	}
	m_hContainerStack.Purge();

	CopySolvers( m_hMainList_Solvers, m_hOutput );

	m_hMainList_Solvers.Purge();
	m_hMainList.Purge();
}
void CopySolvers( const CUtlVector< CHLSL_SolverBase* > &m_hInput,
					CUtlVector< CHLSL_SolverBase* > &m_hOutput )
{
	CUtlVector< CHLSL_SolverBase* > m_hMainList_Solvers;
	CUtlVector< CHLSL_Var* > m_hMainList_Variables;
	m_hOutput.Purge();

	for ( int b = 0; b < m_hInput.Count(); b++ )
	{
		CHLSL_SolverBase *pS = m_hInput[ b ];
		for ( int c = 0; c < pS->GetNumTargetVars(); c++ )
		{
			CHLSL_Var *pV = pS->GetTargetVar( c );
			if ( m_hMainList_Variables.IsValidIndex( m_hMainList_Variables.Find( pV ) ) )
				continue;

			m_hMainList_Variables.AddToTail( pV );
		}
		for ( int c = 0; c < pS->GetNumSourceVars(); c++ )
		{
			CHLSL_Var *pV = pS->GetSourceVar( c );
			if ( m_hMainList_Variables.IsValidIndex( m_hMainList_Variables.Find( pV ) ) )
				continue;

			m_hMainList_Variables.AddToTail( pV );
		}
		if ( m_hMainList_Solvers.IsValidIndex( m_hMainList_Solvers.Find( pS ) ) )
			continue;

		m_hMainList_Solvers.AddToTail( pS );
	}

	CUtlVector< CHLSL_SolverBase* > m_hCopied_Solvers;
	CUtlVector< CHLSL_Var* > m_hCopied_Variables;
#if 1
	for ( int a = 0; a < m_hMainList_Solvers.Count(); a++ )
	{
		CHLSL_SolverBase *pNewSolver = m_hMainList_Solvers[a]->Copy();
		m_hCopied_Solvers.AddToTail( pNewSolver );
	}
	for ( int a = 0; a < m_hMainList_Variables.Count(); a++ )
	{
		CHLSL_Var *pNewVar = new CHLSL_Var( *(m_hMainList_Variables[a]) );
		m_hCopied_Variables.AddToTail( pNewVar );
	}
	//Msg("we counted %i VARS.\n", m_hMainList_Variables.Count());
	Assert( m_hCopied_Solvers.Count() == m_hMainList_Solvers.Count() );
	Assert( m_hCopied_Variables.Count() == m_hMainList_Variables.Count() );
	for ( int a = 0; a < m_hCopied_Solvers.Count(); a++ )
		m_hCopied_Solvers[ a ]->ReconnectVariables( m_hMainList_Solvers[a], m_hMainList_Variables, m_hCopied_Variables );
#endif

	m_hOutput.AddVectorToTail( m_hCopied_Solvers );

	m_hMainList_Solvers.Purge();
	m_hMainList_Variables.Purge();
	m_hCopied_Solvers.Purge();
	m_hCopied_Variables.Purge();
}

inline void ClipToScreenBounds( int &_x, int &_y, int &_sx, int &_sy )
{
	int w,t;
	engine->GetScreenSize( w, t );

	// move
	if ( ( _x + _sx ) > w )
		_x = w - _sx;
	if ( ( _y + _sy ) > t )
		_y = t - _sy;

	// clamp
	_x = max( _x, 0 );
	_y = max( _y, 0 );

	// clamp
	if ( ( _x + _sx ) > w )
		_sx = w;
	if ( ( _y + _sy ) > t )
		_sy = t;
}

static const char *VarTypeName[][2] =
{
	"Float1","float",
	"Float2","float2",
	"Float3","float3",
	"Float4","float4",
	"Matrix 3x3","float3x3",
	"Matrix 4x3","float4x3",
	"Matrix 4x4","float4x4",

	"Sampler","sampler",

	"RESERVED","RESERVED",
	"RESERVED","RESERVED",
	"RESERVED","RESERVED",
	"RESERVED","RESERVED",
	"RESERVED","RESERVED",
	"RESERVED","RESERVED",
	"RESERVED","RESERVED",
	"RESERVED","RESERVED",

	"Master","nocode",
	"Material","nocode",
	"RT","nocode",
};
const char *GetVarTypeName( int num )
{
	return VarTypeName[ num ][ 0 ];
}
const char *GetVarTypeNameCode( int num )
{
	return VarTypeName[ num ][ 1 ];
}
static const char *ResourceTypeName[RESOURCETYPE_][3] =
{
	"VARIABLE","VARIABLE","VARIABLE",
	"vPos","POSITION","float3",
	"vProjPos","POSITION","float4",
	"vNormal","NORMAL","float3",
	"vNormal","NORMAL","float4",
	"vTangent_S","TANGENT","float3",
	"vTangent_S","TANGENT","float4",
	"vTangent_T","BINORMAL","float3",
	"vFlexDelta","POSITION1","float3",
	"vFlexDelta_Normal","NORMAL1","float3",
	"vBoneWeights","BLENDWEIGHT","float4",
	"vBoneIndices","BLENDINDICES","float4",
	"vTexCoord_0","TEXCOORD0","float2",
	"vTexCoord_1","TEXCOORD1","float2",
	"vTexCoord_2","TEXCOORD2","float2",
	"vTexCoord_3","TEXCOORD3","float2",
	"vTexCoord_4","TEXCOORD4","float2",
	"vTexCoord_5","TEXCOORD5","float2",
	"vTexCoord_6","TEXCOORD6","float2",
	"vTexCoord_7","TEXCOORD7","float2",
	"vColor_0","COLOR0","float4",
	"vColor_1","COLOR1","float4",
	"vColor_2","COLOR2","float4",
	"vColor_3","COLOR3","float4",
	"vDepth","DEPTH","float",
	"DUMMY","DUMMY","DUMMY",
};
const char *GetResourceTypeName( int num )
{
	return ResourceTypeName[ num ][ 0 ];
}
const char *GetSemanticTypeName( int num )
{
	return ResourceTypeName[ num ][ 1 ];
}
const char *GetSemanticType( int num )
{
	return ResourceTypeName[ num ][ 2 ];
}
int GetVarTypeRowsRequired( int num )
{
	if ( num <= 3 )
		return 1;
	else if ( num <= 5 )
		return 3;
	else if ( num <= 6 )
		return 4;
	return 1;
}
int GetVarTypeFlag( int num )
{
	return ( 1 << num );
}
int GetVarFlagsVarValue( int flag )
{
	for ( int i = 0; i < HLSLVAR_COUNT; i++ )
		if ( flag & ( 1 << i ) )
			return i;
	return 0;
}
const char *GetVarCodeNameFromFlag( int flag )
{
	return GetVarTypeNameCode(GetVarFlagsVarValue(flag));
}

int GetVarFlagsRowsRequired( int flag )
{
	return GetVarTypeRowsRequired( GetVarFlagsVarValue( flag ) );
}

static const char *textype2name[HLSLTEX_] =
{
	"custom parameter",

	"Basetexture",
	"Bumpmap",
	"Lightmap",
	"Lightmap bumpmap",
	"Framebuffer",
	"Envmap",

	"black",
	"white",
	"grey",

	"flashlight cookie",
	"flashlight depth",
	"flashlight random",

	"morph",
};
const char *GetTextureTypeName( int idx )
{
	Assert( idx >= 0 && idx < HLSLTEX_ );

	return textype2name[ idx ];
}

static const char *itrcodestring[ITERATORCOND_] =
{
	"<",
	">",
	"<=",
	">=",
	"==",
};
const char *GetConditionCodeString( int idx )
{
	Assert( idx >= 0 && idx < 5 );
	return itrcodestring[ idx ];
}
static const char *codetonemapname[] =
{
	"TONEMAP_SCALE_NONE",
	"TONEMAP_SCALE_LINEAR",
	"TONEMAP_SCALE_GAMMA",
};
const char *GetCodeTonemapMacro( int idx )
{
	Assert( idx >= 0 && idx < 3 );
	return codetonemapname[idx];
}
static const char *codelightscalename[] =
{
	"LINEAR_LIGHT_SCALE",
	"LIGHT_MAP_SCALE",
	"ENV_MAP_SCALE",
	"GAMMA_LIGHT_SCALE",
	"g_cLightmapRGB",
};
const char *GetLightscaleCodeString( int idx )
{
	Assert( idx >= 0 && idx < 4 );
	return codelightscalename[ idx ];
}
static const char *gamelightscalename[] =
{
	"Linear",
	"Lightmap",
	"Envmap",
	"Gamma",
	"Lightmap RGB",
};
const char *GetLightscaleGameString( int idx )
{
	Assert( idx >= 0 && idx < 4 );
	return gamelightscalename[ idx ];
}

static const sampinfo_t _samplerdatastruct[ TEXSAMP_MAX ] =
{
	{ "Auto select", "ERROR", 0, false, "X" },

	{ "tex1D", "tex1D", 0, false, "U" },
	{ "tex1Ddd", "tex1D", 0, true, "U" },
	{ "tex1Dbias", "tex1Dbias", 3, false, "U__-t" },
	{ "tex1Dgrad", "tex1Dgrad", 0, true, "U" },
	{ "tex1Dlod", "tex1Dlod", 3, false, "U__-t" },
	{ "tex1Dproj", "tex1Dproj", 3, false, "U__-t" },

	{ "tex2D", "tex2D", 1, false, "UV" },
	{ "tex2Ddd", "tex2D", 1, true, "UV" },
	{ "tex2Dbias", "tex2Dbias", 3, false, "UV_-t" },
	{ "tex2Dgrad", "tex2Dgrad", 1, true, "UV" },
	{ "tex2Dlod", "tex2Dlod", 3, false, "UV_-t" },
	{ "tex2Dproj", "tex2Dproj", 3, false, "UV_-t" },

	{ "tex3D", "tex3D", 2, false, "UVW" },
	{ "tex3Ddd", "tex3D", 2, true, "UVW" },
	{ "tex3Dbias", "tex3Dbias", 3, false, "UVW-t" },
	{ "tex3Dgrad", "tex3Dgrad", 2, true, "UVW" },
	{ "tex3Dlod", "tex3Dlod", 3, false, "UVW-t" },
	{ "tex3Dproj", "tex3Dproj", 3, false, "UVW-t" },

	{ "texCUBE", "texCUBE", 2, false, "Dir" },
	{ "texCUBEdd", "texCUBE", 2, true, "Dir" },
	{ "texCUBEbias", "texCUBEbias", 3, false, "Dir-t" },
	{ "texCUBEgrad", "texCUBEgrad", 2, true, "Dir" },
	{ "texCUBElod", "texCUBElod", 3, false, "Dir-t" },
	{ "texCUBEproj", "texCUBEproj", 3, false, "Dir-t" },
};
const char *GetSamplerData_VisualName( const int &idx )
{
	Assert( idx >= 0 && idx < TEXSAMP_MAX );
	return _samplerdatastruct[idx].sz_visualName;
}
const char *GetSamplerData_CodeName( const int &idx )
{
	Assert( idx >= 0 && idx < TEXSAMP_MAX );
	return _samplerdatastruct[idx].sz_codeName;
}
const int GetSampleData_ComponentSize( const int &idx )
{
	Assert( idx >= 0 && idx < TEXSAMP_MAX );
	return _samplerdatastruct[idx].i_componentSize;
}
const bool GetSamplerData_UseDerivative( const int &idx )
{
	Assert( idx >= 0 && idx < TEXSAMP_MAX );
	return _samplerdatastruct[idx].b_derivativeInput;
}
const char *GetSamplerData_JackName( const int &idx )
{
	Assert( idx >= 0 && idx < TEXSAMP_MAX );
	return _samplerdatastruct[idx].sz_jackname;
}

static const char *fodpathcontexts[ FODPC_ ] = {
	"fod_vtf",
	"fod_mdl",
	"fod_canvas",
	"fod_dump",
	"fod_ufunc",
	"fod_vmt",
};
const char *GetFODPathContext( int type )
{
	Assert( type >= 0 && type < FODPC_ );
	return fodpathcontexts[ type ];
}

static const char *szComboNameList[HLSLCOMBO_MAX] = {
	"ERROR_INVALID_COMBO",
	"ERROR_CUSTOM_COMBO",

	"SKINNING",
	"MORPHING",

	"STATIC_LIGHT",
	"DYNAMIC_LIGHT",
	"NUM_LIGHTS",

	"PIXELFOGTYPE",
	"WRITEWATERFOGTODESTALPHA",

	"FLASHLIGHT",
	"FLASHLIGHTDEPTHFILTERMODE",
	"FLASHLIGHTSHADOWS",

	"COMPRESSED_VERTS",
};
const char *GetComboNameByID( const int &ID )
{
	Assert( HLSLCOMBO_MAX == ARRAYSIZE( szComboNameList ) );

	Assert( ID > HLSLCOMBO_CUSTOM && ID < HLSLCOMBO_MAX );
	return szComboNameList[ ID ];
}
const int GetComboIDByName( const char *name )
{
	Assert( HLSLCOMBO_MAX == ARRAYSIZE( szComboNameList ) );

	for ( int i = 0; i < HLSLCOMBO_MAX; i++ )
		if ( !Q_stricmp( name, szComboNameList[ i ] ) )
			return i;
	return HLSLCOMBO_INVALID;
}
void UpdateComboDataByID( SimpleCombo *pCombo, const int &ID, bool bTotalInit )
{
	Assert( HLSLCOMBO_MAX == ARRAYSIZE( szComboNameList ) );

	int i_min = -1;
	int i_max = -1;
	bool b_static = false;
	bool b_InPreviewMode = false;

	switch ( ID )
	{
	case HLSLCOMBO_FLASHLIGHT_ENABLED:
		i_min = 0;
		i_max = 1;
		b_static = true;
		break;
	case HLSLCOMBO_NUM_LIGHTS:
		i_min = 0;
		i_max = 4;
		b_static = false;
		break;
	case HLSLCOMBO_VERTEXCOMPRESSION:
		b_InPreviewMode = true;
	case HLSLCOMBO_MORPHING:
	case HLSLCOMBO_SKINNING:
	case HLSLCOMBO_LIGHT_STATIC:
	case HLSLCOMBO_LIGHT_DYNAMIC:
	case HLSLCOMBO_PIXELFOG:
	case HLSLCOMBO_WATERFOG_TOALPHA:
	case HLSLCOMBO_FLASHLIGHT_DO_SHADOWS:
		i_min = 0;
		i_max = 1;
		b_static = false;
		break;
	case HLSLCOMBO_FLASHLIGHT_FILTER_MODE:
		i_min = 0;
		i_max = 2;
		b_static = true;
		break;
	default:
		Assert(0);
	}

	if ( ID >= 0 )
	{
		pCombo->iComboType = ID;
		if ( ID > HLSLCOMBO_CUSTOM )
		{
			if ( pCombo->name )
				delete [] pCombo->name;

			const char *szNewName = GetComboNameByID( ID );
			pCombo->name = new char[Q_strlen(szNewName) + 1];
			Q_strcpy( pCombo->name, szNewName );
		}
	}

	if ( bTotalInit )
	{
		if ( i_min >= 0 )
			pCombo->min = i_min;
		if ( i_max >= 0 )
			pCombo->max = i_max;

		pCombo->bInPreviewMode = b_InPreviewMode;
	}
}
void UpdateComboDataByString( SimpleCombo *pCombo, const char *szName, bool bTotalInit )
{
	int ID = GetComboIDByName( szName );
	if ( ID > HLSLCOMBO_INVALID )
		UpdateComboDataByID( pCombo, ID, bTotalInit );
}
SimpleCombo *AllocateComboDataByID( const int &ID )
{
	SimpleCombo *c = new SimpleCombo();
	UpdateComboDataByID( c, ID, true );
	return c;
}
const bool IsComboAvailableInPreviewMode( SimpleCombo *c )
{
	if ( !c )
		return false;

	if ( c->iComboType == HLSLCOMBO_VERTEXCOMPRESSION )
		return true;

	return false;
}
void RemoveAllNonPreviewCombos( CUtlVector< SimpleCombo* >&list )
{
	for ( int i = 0; i < list.Count(); i++ )
	{
		if ( IsComboAvailableInPreviewMode( list[i] ) )
			continue;

		delete list[i];
		list.Remove( i );
		i--;
	}
}


GenericShaderData::GenericShaderData()
{
	AllocCheck_Alloc();

	name = NULL;
	//bPreview = false;
	shader = new BasicShaderCfg_t();
	pUNDEF_Identifiers = new IdentifierLists_t();
	iStackIdx = 0;
}
GenericShaderData::~GenericShaderData()
{
	AllocCheck_Free();

	delete [] name;
	delete shader;
	delete pUNDEF_Identifiers;
}
GenericShaderData::GenericShaderData( const GenericShaderData &o )
{
	AllocCheck_Alloc();

	name = NULL;
	VS_IN = SETUP_HLSL_VS_Input( o.VS_IN );
	VS_to_PS = SETUP_HLSL_VS_Output_PS_Input( o.VS_to_PS );
	PS_OUT = SETUP_HLSL_PS_Output( o.PS_OUT );

	//bPreview = o.bPreview;
	iStackIdx = o.iStackIdx;
	shader = NULL;
	if ( o.shader )
		shader = new BasicShaderCfg_t( *o.shader );

	AutoCopyStringPtr( o.name, &name );

	pUNDEF_Identifiers = NULL;
	if ( o.pUNDEF_Identifiers )
		pUNDEF_Identifiers = new IdentifierLists_t( *o.pUNDEF_Identifiers );
}
bool GenericShaderData::IsPreview()
{
	Assert( shader != NULL );
	return shader->bPreviewMode;
}



GenericPPEData::GenericPPEData()
{
	AllocCheck_Alloc();

	bDoAutoUpdateFBCopy = true;
}
GenericPPEData::~GenericPPEData()
{
	AllocCheck_Free();
}

Preview2DContext::Preview2DContext()
{
	pRenderContext = NULL;

	pMat_PsIn = NULL;
	pMat_PsInOP = NULL;
}

SolverResources::SolverResources( int HierachyType )
{
	Q_memset( this, 0, sizeof( SolverResources ) );
	m_iHierachyType = HierachyType;

	mx_Model.Identity();
	mx_View.Identity();
	mx_Proj.Identity();
}
const int SolverResources::GetHierachyType()
{
	return m_iHierachyType;
}
bool SolverResources::IsVertexShader()
{
	return ( m_iHierachyType == HLSLHIERACHY_VS );
}

static const int __FloatSize = sizeof( float );
inline void AutoCopyFloats( const void *src, void *dst, const int amt )
{
	Q_memcpy( dst, src, __FloatSize * amt );
}
inline void AutoCopyStringPtr( const char *src, char **dst )
{
	*dst = NULL;
	if ( !src || !*src )
		return;

	int len = Q_strlen( src ) + 1;
	*dst = new char[len];
	Q_snprintf( *dst, len, "%s", src );
	(*dst)[len-1] = '\0';
}

//inline int GetChannelNumFromChar( const char *c )
//{
//	switch ( *c )
//	{
//	case 'r':
//	case 'R':
//	case 'x':
//	case 'X':
//			return 0;
//		break;
//	case 'g':
//	case 'G':
//	case 'y':
//	case 'Y':
//			return 1;
//		break;
//	case 'b':
//	case 'B':
//	case 'z':
//	case 'Z':
//			return 2;
//		break;
//	case 'a':
//	case 'A':
//	case 'w':
//	case 'W':
//			return 3;
//		break;
//	}
//	return -1;
//}
//inline char GetCharFromChannelNum( const int i )
//{
//	switch (i)
//	{
//	case 0:
//		return 'x';
//	case 1:
//		return 'y';
//	case 2:
//		return 'z';
//	case 3:
//		return 'w';
//	}
//	return 'x';
//}


inline int GetSlotsFromTypeFlag( int flag )
{
	switch ( flag )
	{
	default:
		Assert(0);
		break;
	case HLSLVAR_FLOAT1:
		return 1;
	case HLSLVAR_FLOAT2:
		return 2;
	case HLSLVAR_FLOAT3:
		return 3;
	case HLSLVAR_FLOAT4:
		return 4;
	case HLSLVAR_MATRIX3X3:
		return 9;
	case HLSLVAR_MATRIX4X3:
		return 12;
	case HLSLVAR_MATRIX4X4:
		return 16;
	}
	return 1;
}
inline int GetTypeFlagFromEnum( int i )
{
	switch ( i )
	{
	default:
		Assert(0);
		break;
	case 1:
		return HLSLVAR_FLOAT1;
	case 2:
		return HLSLVAR_FLOAT2;
	case 3:
		return HLSLVAR_FLOAT3;
	case 4:
		return HLSLVAR_FLOAT4;

	case 5:
		return HLSLVAR_MATRIX3X3;
	case 6:
		return HLSLVAR_MATRIX4X3;
	case 7:
		return HLSLVAR_MATRIX4X4;

	case 8:
		//Assert(0);
		return HLSLVAR_SAMPLER;
	}
	return HLSLVAR_FLOAT1;
}

int StringTabsWorth( const char *text )
{
	return Q_strlen( text ) / 4;
}

static const LimitReport_t limitreportdefaults[] =
{
	{ "PS samplers", 0, AMT_PS_SAMPLERS },
	{ "PS constants", 0, AMT_PS_CREG },

	{ "VS samplers", 0, AMT_VS_SAMPLERS },
	{ "VS constants", 0, AMT_VS_CREG },

	{ "Vmt static", 0, AMT_VMT_STATIC },
	{ "Vmt mutable", 0, AMT_VMT_MUTABLE },

	{ "Texture mismatch", 0, -1 },
};
const LimitReport_t &GetLimitReporterDefault( int i )
{
	Assert( i >= 0 && i < LERROR_ );
	return limitreportdefaults[i];
}
bool ReportErrors( GenericShaderData *data, CUtlVector< LimitReport_t* > &hErrorList )
{
	bool bError = false;
	LimitReport_t *report = NULL;

	for ( int i = LERROR__FIRST; i <= LERROR__LAST; i++ )
	{
		Assert( report == NULL );
		report = new LimitReport_t( GetLimitReporterDefault( i ) );
		AllocCheck_AllocS("LimitReport_t");

		int count_current = 0;

		switch ( i )
		{
		case LERROR_PS_SAMPLER:
				count_current = data->shader->pPS_Identifiers->hList_Textures.Count();
			break;
		case LERROR_VS_SAMPLER:
				count_current = data->shader->pVS_Identifiers->hList_Textures.Count();
			break;
		case LERROR_PS_CONSTS:
		case LERROR_VS_CONSTS:
			{
				const bool bPS = i == LERROR_PS_CONSTS;
				IdentifierLists_t *pList = bPS ? data->shader->pPS_Identifiers : data->shader->pVS_Identifiers;
				if ( !pList )
					break;

				for ( int a = 0; a < pList->hList_EConstants.Count(); a++ )
				{
					int curSize = pList->hList_EConstants[a]->iConstSize;
					if ( !HLSLENV_SHOULD_COUNT_CONST( pList->hList_EConstants[a]->iEnvC_ID, bPS ) )
						curSize = 0;
					count_current += curSize;
				}
			}
			break;
		case LERROR_VMT_PARAMS_STATIC:
		case LERROR_VMT_PARAMS_MUTABLE:
			{
				const int TargetEnvID = ( i == LERROR_VMT_PARAMS_STATIC ) ? HLSLENV_SMART_VMT_STATIC : HLSLENV_SMART_VMT_MUTABLE;

				CUtlVector< SimpleEnvConstant* >hListedConstants;
				for ( int a = 0; a < 2; a++ )
				{
					IdentifierLists_t *pList = a ? data->shader->pPS_Identifiers : data->shader->pVS_Identifiers;
					if ( !pList )
						continue;
					for ( int b = 0; b < pList->hList_EConstants.Count(); b++ )
					{
						if ( pList->hList_EConstants[b]->iEnvC_ID != TargetEnvID )
							continue;
						bool bContinue = false;
						for ( int c = 0; c < hListedConstants.Count(); c++ )
						{
							SimpleEnvConstant *pConstLast = hListedConstants[c];
							Assert( pConstLast->szSmartHelper && pList->hList_EConstants[b]->szSmartHelper );

							if ( !Q_stricmp( pConstLast->szSmartHelper, pList->hList_EConstants[b]->szSmartHelper ) )
								bContinue = true;
						}
						if ( bContinue )
							continue;

						hListedConstants.AddToTail( pList->hList_EConstants[b] );
						count_current++;
					}
				}
				hListedConstants.Purge();
			}
			break;
		case LERROR_GENERIC_DEMOTEXTURE_MISMTACH:
			{
				//cutlvector< simpletexture* >&htexlist = data->shader->pps_identifiers->hlist_textures;
				//for ( int a = 0; a < htexlist.count(); a++ )
				//{
				//	simpletexture &_tex_a = htexlist[a];
				//	for ( int b = 0; b < htexlist.count(); b++ )
				//	{
				//		simpletexture &_tex_b = htexlist[b];
				//		if ( a == b )
				//			continue;
				//		

				//	}
				//}
			}
			break;
		}

		report->i_cur = count_current;
		hErrorList.AddToTail( report );

		bError = bError || (report->i_cur > report->i_max);
		report = NULL;
	}
	return bError;
}

WriteContext_FXC::WriteContext_FXC()
{
	buf_combos.SetBufferType( true, true );
	buf_samplers.SetBufferType( true, true );
	buf_constants.SetBufferType( true, true );
	buf_arrays.SetBufferType( true, true );
	buf_functions_globals.SetBufferType( true, true );
	buf_functions_bodies.SetBufferType( true, true );
	buf_semantics_In.SetBufferType( true, true );
	buf_semantics_Out.SetBufferType( true, true );
	buf_code.SetBufferType( true, true );

	buf_combos.EnableDirectives( true );
	buf_samplers.EnableDirectives( true );
	buf_constants.EnableDirectives( true );
	buf_arrays.EnableDirectives( true );
	buf_functions_globals.EnableDirectives( true );
	buf_functions_bodies.EnableDirectives( true );
	buf_semantics_In.EnableDirectives( true );
	buf_semantics_Out.EnableDirectives( true );
	buf_code.EnableDirectives( true );

	bPreview = true;

	m_hActive_Solvers = NULL;
	m_pActive_Identifiers = NULL;
}

RunCodeContext::RunCodeContext( bool bPreview, bool bScene )
{
	pRenderContext = NULL;
	bPreviewMode = bPreview;
	bSceneMode = bScene;
}
const bool &RunCodeContext::IsUsingPreview() const
{
	return bPreviewMode;
}
const bool &RunCodeContext::IsSceneMode() const
{
	return bSceneMode;
}

void SetupForRendering( CUtlVector< CHLSL_SolverBase* > &hStack )
{
	for ( int i = 0; i < hStack.Count(); i++ )
		hStack[ i ]->Invoke_RenderInit();
}

void UniquifyComboList( CUtlVector< SimpleCombo* > &_combos )
{
	CUtlVector< SimpleCombo* > _newlist;
	for ( int i = 0; i < _combos.Count(); i++ )
	{
		SimpleCombo *c = _combos[i];
		bool bAdd = true;
		for ( int x = 0; x < _newlist.Count(); x++ )
		{
			SimpleCombo *n = _newlist[x];
			if ( !Q_stricmp( c->name, n->name ) /*&&
				c->bStatic == n->bStatic*/ )
			{
				bAdd = false;
				if ( n->iComboType <= HLSLCOMBO_CUSTOM )
					n->iComboType = c->iComboType;
				n->max = max( n->max, c->max );
				n->bStatic = n->bStatic || c->bStatic;
			}
		}
		if ( !bAdd )
		{
			delete c;
			continue;
		}
		_newlist.AddToTail( c );
	}
	_combos.Purge();
	_combos.AddVectorToTail( _newlist );
	_newlist.Purge();
}
void UniquifyTextureList( CUtlVector< SimpleTexture* > &_tex )
{
	CUtlVector< SimpleTexture* > _newlist;
	for ( int i = 0; i < _tex.Count(); i++ )
	{
		SimpleTexture *cur = _tex[i];
		bool bAdd = true;
		CUtlVector< HNODE* >hNodesToAdd;
		for ( int a = 0; a < _newlist.Count(); a++ )
		{
			SimpleTexture *output = _newlist[a];
			if ( output->iTextureMode != cur->iTextureMode )
				continue;
			if ( cur->iTextureMode == HLSLTEX_CUSTOMPARAM &&
				Q_stricmp( cur->szParamName, output->szParamName ) )
				continue;

			bAdd = false;
			output->m_hTargetNodes.AddVectorToTail( cur->m_hTargetNodes );
			cur->m_hTargetNodes.Purge();
		}
		if ( !bAdd )
		{
			delete cur;
			continue;
		}

		cur->iSamplerIndex = _newlist.Count();
		_newlist.AddToTail( cur );
	}
	_tex.Purge();
	_tex.AddVectorToTail( _newlist );
	_newlist.Purge();
}
//bool CountConstant( int envcid, bool bPS )
//{
//	if ( envcid <= HLSLENV_FOG_PARAMS )
//		return true;
//	if ( !bPS && envcid == HLSLENV_FLASHLIGHT_VPMATRIX )
//		return true;
//	if ( envcid == HLSLENV_SMART_CALLBACK ||
//		envcid == HLSLENV_SMART_VMT_STATIC ||
//		envcid == HLSLENV_SMART_VMT_MUTABLE )
//		return true;
//	return false;
//}
void UniquifyConstantList( bool bPS, CUtlVector< SimpleEnvConstant* > &_const )
{
	CUtlVector< SimpleEnvConstant* > _newlist;
	int currentNormalizedNum = 0;
	for ( int i = 0; i < _const.Count(); i++ )
	{
		SimpleEnvConstant *cur = _const[i];
		bool bAdd = true;
		for ( int a = 0; a < _newlist.Count(); a++ )
		{
			SimpleEnvConstant *output = _newlist[a];
			if ( output->iEnvC_ID != cur->iEnvC_ID )
				continue;

			if ( HLSLENV_IS_SMART_CONST( output->iEnvC_ID ) )
			{
				Assert( output->szSmartHelper && cur->szSmartHelper );
				if ( Q_stricmp( output->szSmartHelper, cur->szSmartHelper ) )
					continue;
			}

			if ( output->iEnvC_ID == HLSLENV_CUSTOM_MATRIX &&
				output->iSmartNumComps != cur->iSmartNumComps )
				continue;

			bAdd = false;
		}
		if ( !bAdd )
		{
			delete cur;
			continue;
		}

		Assert( cur->iConstSize > 0 );

		if ( HLSLENV_SHOULD_COUNT_CONST( cur->iEnvC_ID, bPS ) )
		{
			cur->iHLSLRegister = currentNormalizedNum;
			currentNormalizedNum += cur->iConstSize;
		}

		_newlist.AddToTail( cur );
	}
	_const.Purge();
	_const.AddVectorToTail( _newlist );
	_newlist.Purge();
}
void UniquifyFunctionList( CUtlVector< SimpleFunction* > &_funcs )
{
	CUtlVector< SimpleFunction* > _newlist;
	for ( int i = 0; i < _funcs.Count(); i++ )
	{
		SimpleFunction *f = _funcs[i];
		bool bAdd = true;
		for ( int x = 0; x < _newlist.Count(); x++ )
		{
			SimpleFunction *n = _newlist[x];

			Assert( f->szFuncName && n->szFuncName );
			bool bSameFunctionName = !Q_strcmp( f->szFuncName, n->szFuncName );
			bool bSameParams = f->hParams.Count() == n->hParams.Count();
			if ( bSameParams )
			{
				for ( int p = 0; p < f->hParams.Count(); p++ )
				{
					if ( f->hParams[p] != n->hParams[p] )
						bSameParams = false;
				}
			}

			if ( bSameFunctionName && bSameParams )
			{
				bAdd = false;
			}
		}
		if ( !bAdd )
		{
			delete f;
			continue;
		}
		_newlist.AddToTail( f );
	}
	_funcs.Purge();
	_funcs.AddVectorToTail( _newlist );
	_newlist.Purge();
}
void ClearSingleIdentifier( IdentifierLists_t *id )
{
	SaveDeleteVector( id->hList_Combos );
	SaveDeleteVector( id->hList_Textures );
	SaveDeleteVector( id->hList_EConstants );
	SaveDeleteVector( id->hList_Arrays );
	SaveDeleteVector( id->hList_Functions );
}
void ClearIdentifiers( GenericShaderData &data )
{
	ClearSingleIdentifier( data.shader->pVS_Identifiers );
	ClearSingleIdentifier( data.shader->pPS_Identifiers );
	ClearSingleIdentifier( data.pUNDEF_Identifiers );
}
void AllocateIdentifiers( IdentifierLists_t &Identifiers, CUtlVector< CHLSL_SolverBase* > &hStack )
{
	for ( int i = 0; i < hStack.Count(); i++ )
	{
		hStack[ i ]->Invoke_IdentifierAllocate( Identifiers );
	}
}
void UniquifyIdentifiers( bool bPS, IdentifierLists_t &Identifiers )
{
	UniquifyComboList( Identifiers.hList_Combos );
	UniquifyTextureList( Identifiers.hList_Textures );
	UniquifyConstantList( bPS, Identifiers.hList_EConstants );
	UniquifyFunctionList( Identifiers.hList_Functions );

	if ( !bPS )
	{
		SimpleTexture *pMorph = NULL;
		for (int i = 0; i < Identifiers.hList_Textures.Count() && !pMorph; i++ )
			if ( Identifiers.hList_Textures[i]->iTextureMode == HLSLTEX_MORPH )
				pMorph = Identifiers.hList_Textures[i];

		if ( pMorph )
		{
			Identifiers.hList_Textures.FindAndRemove( pMorph );
			Identifiers.hList_Textures.AddToTail( pMorph );
			for (int i = 0; i < Identifiers.hList_Textures.Count(); i++ )
				Identifiers.hList_Textures[i]->iSamplerIndex = i;
		}
	}
}
void CalcNumDynamicCombos( IdentifierLists_t &Identifiers, bool bPreview )
{
	Identifiers.inum_DynamicCombos = 1;
	for ( int i = 0; i < Identifiers.hList_Combos.Count(); i++ )
	{
		if ( Identifiers.hList_Combos[i]->bStatic )
			continue;

		if ( bPreview && !Identifiers.hList_Combos[i]->bInPreviewMode )
			continue;

		int curAmt = Identifiers.hList_Combos[i]->GetAmt();
		Identifiers.inum_DynamicCombos *= curAmt;
	}
}

#define GAUSS_BIAS_LIMIT (0.99999f)
float GaussianWeight_1D_Internal( const int &iSample_pos, const int &iKernel_size, const float &flBias )
{
	const long double flDistribution = ( ( iKernel_size + 1 ) / 6.0f ) - flBias;
	const long double flPos = ( (float)iSample_pos ) - ( ( ( iKernel_size - 1.0f ) / 2.0f ) );

	long double flWeight = 1.0f / ( sqrt( 2.0f * M_PI_F ) * flDistribution );
	flWeight *= exp( -1.0f * ( pow( flPos, 2 ) / ( 2.0f * pow( flDistribution, 2 ) ) ) );
	return flWeight;
}
float GaussianWeight_1D( const int &iSample_pos, const int &iKernel_size, const float &flBias )
{
	const float flBiasClamped = clamp( flBias, -GAUSS_BIAS_LIMIT, GAUSS_BIAS_LIMIT );
	long double flAccum = 0;
	for ( int i = 0; i < iKernel_size; i++ )
		flAccum += GaussianWeight_1D_Internal( i, iKernel_size, flBiasClamped );

	long double flRemainder = ( 1.0f - flAccum );
	long double flWeight = GaussianWeight_1D_Internal( iSample_pos, iKernel_size, flBiasClamped );
	float flFinalWeight = flWeight + ( flRemainder * ( flWeight / flAccum ) );
	return flFinalWeight;
}

float GaussianWeight_2D_Internal( const int &iPos_X, const int &iPos_Y,
	const int &iSize_X, const int &iSize_Y,
	const float &flBias )
{
	const long double flDistribution_x = ( ( iSize_X + 1 ) / 6.0f ) - flBias;
	const long double flPos_x = ( (float)iPos_X ) - ( ( ( iSize_X - 1.0f ) / 2.0f ) );
	const long double flDistribution_y = ( ( iSize_Y + 1 ) / 6.0f ) - flBias;
	const long double flPos_y = ( (float)iPos_Y ) - ( ( ( iSize_Y - 1.0f ) / 2.0f ) );

	const long double center_x = 0;
	const long double center_y = 0;
	const long double amplitude = 1.0f;

	long double step_x = pow( flPos_x - center_x, 2 ) / ( 2.0f * pow( flDistribution_x, 2 ) );
	long double step_y = pow( flPos_y - center_y, 2 ) / ( 2.0f * pow( flDistribution_y, 2 ) );
	long double exponent = -1.0f * ( step_x + step_y );

	return amplitude * exp( exponent );
}
float GaussianWeight_2D( const int &iPos_X, const int &iPos_Y,
	const int &iSize_X, const int &iSize_Y,
	const float &flBias )
{
	const float flBiasClamped = clamp( flBias, -GAUSS_BIAS_LIMIT, GAUSS_BIAS_LIMIT );
	long double flAccum = 0;
	for ( int x = 0; x < iSize_X; x++ )
		for ( int y = 0; y < iSize_Y; y++ )
			flAccum += GaussianWeight_2D_Internal( x, y, iSize_X, iSize_Y, flBiasClamped );

	long double flRemainder = ( 1.0f - flAccum );
	long double flWeight = GaussianWeight_2D_Internal( iPos_X, iPos_Y, iSize_X, iSize_Y, flBiasClamped );
	return flWeight + ( flRemainder * ( flWeight / flAccum ) );
}


//CON_COMMAND( gauss_1d, "pos kernelsize bias" )
//{
//	if ( args.ArgC() < 3 )
//		return;
//
//	int pos = atoi( args[ 1 ] );
//	int size = atoi( args[ 2 ] );
//	float bias = 0;
//	if ( args.ArgC() >= 4 )
//		bias = atof( args[3] );
//
//	float weight = GaussianWeight_1D( pos, size, bias );
//	Msg( "gauss 1d, pos: %i, kernelsize: %i, bias: %f, weight: %f\n", pos, size, bias, weight );
//}
//CON_COMMAND( gauss_2d, "posx posy sizex sizey bias" )
//{
//	if ( args.ArgC() < 5 )
//		return;
//
//	int posx = atoi( args[ 1 ] );
//	int posy = atoi( args[ 2 ] );
//	int sizex = atoi( args[ 3 ] );
//	int sizey = atoi( args[ 4 ] );
//	float bias = 0;
//	if ( args.ArgC() >= 6 )
//		bias = atof( args[5] );
//
//	float weight = GaussianWeight_2D( posx, posy, sizex, sizey, bias );
//	Msg( "gauss 2d, pos: %i/%i, kernelsize: %i/%i, bias: %f, weight: %f\n", posx, posy, sizex, sizey, bias, weight );
//}

void BuildFormatFlags( BasicShaderCfg_t &data, CUtlVector< CHLSL_SolverBase* > &hVertexshaderStack, CUtlVector< CHLSL_SolverBase* > &hPixelhaderStack )
{
	data.iVFMT_flags = VERTEX_POSITION;
#ifndef SHADER_EDITOR_DLL_SWARM
	data.iVFMT_flags |= VERTEX_FORMAT_VERTEX_SHADER;
#endif
	data.iVFMT_numTexcoords = 1;
	data.iVFMT_numUserData = 0;
	data.bRefractionSupport = false;
	data.bVertexLighting = false;

	for (int i = 0; i < hVertexshaderStack.Count(); i++)
	{
		CHLSL_Solver_ReadSemantics *_read = dynamic_cast< CHLSL_Solver_ReadSemantics* >( hVertexshaderStack[i] );
		if ( _read )
		{
			ResourceType_t type = _read->GetResourceType();
			switch ( type )
			{
			case RESOURCETYPE_NORMAL_4:
#ifndef SHADER_EDITOR_DLL_2006
				data.iVFMT_flags |= VERTEX_FORMAT_COMPRESSED;
#endif
#ifdef SHADER_EDITOR_DLL_SWARM
				data.iVFMT_flags |= VERTEX_FORMAT_PAD_POS_NORM;
#endif
			case RESOURCETYPE_NORMAL:
				data.iVFMT_flags |= VERTEX_NORMAL;
				break;

			case RESOURCETYPE_TANGENTS_4:
				data.iVFMT_numUserData = 4;
				//data.iVFMT_flags |= VERTEX_TANGENT_T;
				data.iVFMT_flags |= VERTEX_USERDATA_SIZE( data.iVFMT_numUserData );
				break;
			case RESOURCETYPE_TANGENTS_3:
				data.iVFMT_flags |= VERTEX_TANGENT_S;
				break;
			case RESOURCETYPE_TANGENTT:
				data.iVFMT_flags |= VERTEX_TANGENT_T;
				break;

			case RESOURCETYPE_COLOR_0:
			case RESOURCETYPE_COLOR_1:
			case RESOURCETYPE_COLOR_2:
			case RESOURCETYPE_COLOR_3:
				data.iVFMT_flags |= VERTEX_COLOR;
				break;

			case RESOURCETYPE_FLEXDELTA:
			case RESOURCETYPE_BONE_WEIGHTS:
			case RESOURCETYPE_BONE_INDICES:
				data.iVFMT_flags |= VERTEX_BONE_INDEX;
				data.iVFMT_flags |= VERTEX_BONEWEIGHT( 3 );
				break;

			case RESOURCETYPE_TEXCOORD_2:
			case RESOURCETYPE_TEXCOORD_1:
			case RESOURCETYPE_TEXCOORD_0:
				{
					int texcoordNum = type - RESOURCETYPE_TEXCOORD_0;
					CHLSL_Solver_ReadSemantics *_readVertex = dynamic_cast< CHLSL_Solver_ReadSemantics* >( hVertexshaderStack[i] );

					Assert( _readVertex );
					Assert( texcoordNum >= 0 && texcoordNum < 3 );

					data.iVFMT_texDim[ texcoordNum ] = GetVarFlagsVarValue( _readVertex->GetTexcoordFlag( texcoordNum ) ) + 1;
					data.iVFMT_numTexcoords = max( data.iVFMT_numTexcoords, texcoordNum + 1 );

					data.iVFMT_flags |= VERTEX_TEXCOORD_SIZE( texcoordNum, data.iVFMT_texDim[ texcoordNum ] );
					break;
				}
			}
		}

		CHLSL_Solver_StdVLight *_vlight = dynamic_cast< CHLSL_Solver_StdVLight* >( hVertexshaderStack[i] );
		if ( _vlight )
		{
			data.bVertexLighting = true;
		}
	}
	for (int i = 0; i < hPixelhaderStack.Count(); i++)
	{
		CHLSL_Solver_TextureSample *_texSample = dynamic_cast< CHLSL_Solver_TextureSample* >( hPixelhaderStack[i] );
		if ( _texSample )
		{
			if ( _texSample->GetTextureMode() == HLSLTEX_FRAMEBUFFER )
				data.bRefractionSupport = true;
		}

		CHLSL_Solver_StdMorph *_morph = dynamic_cast< CHLSL_Solver_StdMorph* >( hPixelhaderStack[i] );
		if ( _morph )
		{
			//data.iVFMT_numTexcoords = 3;
		}
	}

	//if ( data.iVFMT_numUserData )
	//	data.iVFMT_flags &= ~VERTEX_TANGENT_S;

	if ( data.bVertexLighting )
		data.iVFMT_flags &= ~VERTEX_COLOR;
}

extern CViewSetup_SEdit_Shared _MainView;

void __pSolve_Time( float *_fl4 )
{
	_fl4[ 0 ] = GetEditorTime(); //engine->Time();
}
void __pSolve_Origin( float *_fl4 )
{
	_fl4[ 2 ] = 0.5f;
}
void __pSolve_ViewForward( float *_fl4 )
{
	_fl4[2] = -1;
}
void __pSolve_ViewRight( float *_fl4 )
{
	_fl4[1] = -1;
}
void __pSolve_ViewUp( float *_fl4 )
{
	_fl4[0] = 1;
}
void __pSolve_ViewDepth( float *_fl4 )
{
	_fl4[0] = _MainView.zFar;
}
void __pSolve_PixelSize( float *_fl4 )
{
	_fl4[0] = 0.05f;
	_fl4[1] = 0.05f;
}
void __pSolve_Dummy( float *_fl4 ){}
static econstdata econstinfoarray[] =
{
	{ "_g_Time", "secs", HLSLVAR_FLOAT1, "Time", HLSLNODE_CONSTANT_TIME, &::__pSolve_Time },
	{ "_g_VecOrig", "orig", HLSLVAR_FLOAT3, "View origin", HLSLNODE_CONSTANT_VIEW_ORIGIN, &::__pSolve_Origin },
	{ "_g_VecForward", "fwd", HLSLVAR_FLOAT3, "View forward", HLSLNODE_CONSTANT_VIEW_FORWARD, &::__pSolve_ViewForward },
	{ "_g_VecRight", "right", HLSLVAR_FLOAT3, "View right", HLSLNODE_CONSTANT_VIEW_RIGHT, &::__pSolve_ViewRight },
	{ "_g_VecUp", "up", HLSLVAR_FLOAT3, "View up", HLSLNODE_CONSTANT_VIEW_UP, &::__pSolve_ViewUp },
	{ "_g_WorldDepth", "depth", HLSLVAR_FLOAT1, "View world depth", HLSLNODE_CONSTANT_VIEW_WORLDSPACE_DEPTH, &::__pSolve_ViewDepth },
	{ "_g_TexelSize", "size", HLSLVAR_FLOAT2, "FB texelsize", HLSLNODE_CONSTANT_FB_PIXELSIZE, &::__pSolve_PixelSize },
	{ "_g_FogParams", "data", HLSLVAR_FLOAT4, "Fog params", HLSLNODE_CONSTANT_FOGPARAMS, &::__pSolve_Dummy },
};
econstdata *EConstant_GetData( int i )
{
	Assert( i >= 0 && i <= 7 );
	return &(econstinfoarray[i]);
}

static const customMatrix_t cmatrixData[] = {
	{ "float4x3", "g_cCMatrix_View", "View", 3, HLSLVAR_MATRIX4X3 },
	{ "float4x4", "g_cCMatrix_Proj", "Proj", 4, HLSLVAR_MATRIX4X4 },
	{ "float4x4", "g_cCMatrix_ViewProj", "ViewProj", 4, HLSLVAR_MATRIX4X4 },

	{ "float4x3", "g_cCMatrix_ViewInv", "View Inv", 3, HLSLVAR_MATRIX4X3 },
	{ "float4x4", "g_cCMatrix_ProjInv", "Proj Inv", 4, HLSLVAR_MATRIX4X4 },
	{ "float4x4", "g_cCMatrix_ViewProjInv", "ViewProj Inv", 4, HLSLVAR_MATRIX4X4 },
};
const customMatrix_t *GetCMatrixInfo( int ID )
{
	Assert( ID >= 0 && ID < CMATRIX_LAST );
	return &(cmatrixData[ ID ]);
}

#define NUM_DEFAULT_PUZZLES_PER_ROW 4 //8

int curPuzzleIndex = 0;
int curPuzzlesPerRow = 0;

//ITexture *_tex_PingPong[2] = {0,0};
static int sizeold = 0;
static int puzzlesize = 0;

static bool bAdded = false;
void ReloadCommon( int f )
{
	//pEditorRoot->DeleteProceduralMaterials();
	//pEditorRoot->AllocProceduralMaterials();
	if ( pEditorRoot )
		pEditorRoot->UpdateVariablePointer();
}

//CON_COMMAND( manualrestore, "" )
//{
//	pEditorRoot->UpdateVariablePointer();
//}
static CTextureReference _FFB;
ITexture *GetFBTex()
{
	if ( !_FFB ) //|| IsErrorTexture( _FFB ) )
	{
		_FFB.Init( materials->FindTexture( "_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( _FFB ) );
	}
	return _FFB;
}
static CTextureReference _FFB_Small;
ITexture *GetFBTexSmall()
{
	if ( !_FFB_Small )
	{
		_FFB_Small.Init( materials->FindTexture( "_rt_SmallFB1", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( _FFB_Small ) );
	}
	return _FFB_Small;
}
static CTextureReference _tex_PingPong[3];
ITexture *GetPingPongRT( int num )
{
	if ( !_tex_PingPong[num] )
	{
		_tex_PingPong[num].Init( materials->FindTexture( VarArgs("_rt_SEdit_pingpong_%i",num), TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( _tex_PingPong[num] ) );
	}
	return _tex_PingPong[num];
}
static CTextureReference _teX_fbcopy;
ITexture *GetFBCopyTex()
{
	if ( !_teX_fbcopy )
	{
		_teX_fbcopy.Init( materials->FindTexture( "_rt_SEdit_FramebufferCopy", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( _teX_fbcopy ) );
	}
	return _teX_fbcopy;
}
static CTextureReference _teX_Skymask;
ITexture *GetSkymaskTex()
{
	if ( !_teX_Skymask )
	{
		_teX_Skymask.Init( materials->FindTexture( "_rt_SEdit_Skymask", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( _teX_Skymask ) );
	}
	return _teX_Skymask;
}

static SEDIT_SKYMASK_MODE iSkymask = SKYMASK_OFF;
const bool IsSkymaskAvailable()
{
	return iSkymask != SKYMASK_OFF;
}
const SEDIT_SKYMASK_MODE GetSkymaskMode()
{
	return iSkymask;
}

#define FALLBACK_RTS 1

void AllocRTs( int size )
{
	if ( !bAdded )
	{
		materials->AddRestoreFunc( ReloadCommon );
		bAdded = true;
	}
	if (sizeold == size)
		return;

	//ITexture *tmp = materials->FindTexture( "_rt_SEdit_pingpong_0", TEXTURE_GROUP_OTHER, false );
	//ForceDeleteTexture( &tmp );
	//if ( !IsErrorTexture(tmp) )
	//	tmp->Release();
	//tmp = materials->FindTexture( "_rt_SEdit_pingpong_1", TEXTURE_GROUP_OTHER, false );
	//ForceDeleteTexture( &tmp );
	//if ( !IsErrorTexture(tmp) )
	//	tmp->Release();
	//tmp = materials->FindTexture( "_rt_SEdit_FramebufferCopy", TEXTURE_GROUP_OTHER, false );
	//ForceDeleteTexture( &tmp );
	//if ( !IsErrorTexture(tmp) )
	//	tmp->Release();
	//tmp = materials->FindTexture( "_rt_SEdit_pingpong_2", TEXTURE_GROUP_OTHER, false );
	//ForceDeleteTexture( &tmp );
	//if ( !IsErrorTexture(tmp) )
	//	tmp->Release();

	int flags = TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD |
		TEXTUREFLAGS_RENDERTARGET | TEXTUREFLAGS_NODEPTHBUFFER |
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT;

	ImageFormat RT_format_0 = IMAGE_FORMAT_RGBA32323232F;
#if FALLBACK_RTS
	ImageFormat RT_format_1 = IMAGE_FORMAT_RGBA8888;
#endif


	materials->BeginRenderTargetAllocation();

	ITexture *tmp = NULL;
	tmp = materials->CreateNamedRenderTargetTextureEx( "_rt_SEdit_pingpong_0",
		size, size,
		RT_SIZE_NO_CHANGE,
		RT_format_0,
		MATERIAL_RT_DEPTH_NONE,
		flags, 0 );
#if FALLBACK_RTS
	if ( !tmp )
		tmp = materials->CreateNamedRenderTargetTextureEx( "_rt_SEdit_pingpong_0",
			size, size,
			RT_SIZE_NO_CHANGE,
			RT_format_1,
			MATERIAL_RT_DEPTH_NONE,
			flags, 0 );
#endif

	int size2 = 2; //size;

	tmp = NULL;
	tmp = materials->CreateNamedRenderTargetTextureEx( "_rt_SEdit_pingpong_1",
		size2, size2,
		RT_SIZE_NO_CHANGE,
		RT_format_0,
		MATERIAL_RT_DEPTH_NONE,
		flags, 0 );
#if FALLBACK_RTS
	if ( !tmp )
		tmp = materials->CreateNamedRenderTargetTextureEx( "_rt_SEdit_pingpong_1",
			size2, size2,
			RT_SIZE_NO_CHANGE,
			RT_format_1,
			MATERIAL_RT_DEPTH_NONE,
			flags, 0 );
#endif
	tmp = NULL;
	tmp = materials->CreateNamedRenderTargetTextureEx( "_rt_SEdit_FramebufferCopy",
		128, 128,
		RT_SIZE_FULL_FRAME_BUFFER,
		IMAGE_FORMAT_RGB888,
		MATERIAL_RT_DEPTH_NONE,
		flags, 0 );

	tmp = materials->CreateNamedRenderTargetTextureEx( "_rt_SEdit_pingpong_2",
		size, size,
		RT_SIZE_NO_CHANGE,
		IMAGE_FORMAT_RGB888,
		MATERIAL_RT_DEPTH_NONE,
		flags, 0 );

#if 0
	materials->CreateNamedRenderTargetTextureEx( "_rt_vst",
		128, 128,
		RT_SIZE_OFFSCREEN,
		IMAGE_FORMAT_RGBA16161616F,
		MATERIAL_RT_DEPTH_NONE,
		TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_NODEBUGOVERRIDE |
TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_POINTSAMPLE | TEXTUREFLAGS_VERTEXTEXTURE, 0 );
#endif

#if 0
	ImageFormat imt_dt = materials->GetShadowDepthTextureFormat();
	Warning( "depth tex format: %i\n", imt_dt );
	ITexture *dtex = materials->CreateNamedRenderTargetTextureEx( "_rt_depthtextest", 100, 99, RT_SIZE_NO_CHANGE, imt_dt, MATERIAL_RT_DEPTH_ONLY, flags );
	Warning( "depth tex alloc state: %d\n", (!IsErrorTexture( dtex )) );
	Warning( "depth tex values sanity test: %d\n", ( dtex && dtex->GetActualWidth() == dtex->GetMappingWidth() && dtex->GetActualWidth() == 100 &&
		dtex->GetActualHeight() == dtex->GetMappingHeight() && dtex->GetActualHeight() == 99 ) );
#endif

	materials->EndRenderTargetAllocation();

	Assert( !IsErrorTexture( GetPingPongRT(0) ) );
	Assert( !IsErrorTexture( GetPingPongRT(2) ) );

	Assert( GetPingPongRT(0)->GetMappingWidth() == size && GetPingPongRT(0)->GetMappingHeight() == size );
	Assert( GetPingPongRT(2)->GetMappingWidth() == size && GetPingPongRT(2)->GetMappingHeight() == size );

	Assert( !IsErrorTexture( GetFBCopyTex() ) );

	sizeold = size;
}
void AllocSkymask( SEDIT_SKYMASK_MODE iSkymaskMode )
{
	iSkymask = clamp( iSkymaskMode, SKYMASK_OFF, SKYMASK_FULL );
	if ( !IsSkymaskAvailable() )
		return;

	int flags = TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_RENDERTARGET | TEXTUREFLAGS_NODEPTHBUFFER;

	RenderTargetSizeMode_t rtsizemode = ( iSkymask == SKYMASK_QUARTER ) ? RT_SIZE_HDR : RT_SIZE_FULL_FRAME_BUFFER;

	materials->BeginRenderTargetAllocation();
	{
		materials->CreateNamedRenderTargetTextureEx( "_rt_SEdit_Skymask",
			128, 128,
			rtsizemode,
			IMAGE_FORMAT_RGBA8888,
			MATERIAL_RT_DEPTH_NONE,
			flags | TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, 0 );
	}
	materials->EndRenderTargetAllocation();
}

const int &GetRTPuzzleSize()
{
	return puzzlesize;
}
const int &GetRTSize()
{
	return sizeold;
}

void ResetTileInfo()
{
	Assert( sizeold );
	Assert( !IsErrorTexture( GetPingPongRT(0) ) );

	curPuzzleIndex = 0;
	puzzlesize = sizeold / NUM_DEFAULT_PUZZLES_PER_ROW;
	curPuzzlesPerRow = NUM_DEFAULT_PUZZLES_PER_ROW;
}
void CalcPuzzleSize( int numSolvers )
{
	Assert( sizeold );
	while ( (numSolvers-1) > (curPuzzlesPerRow*curPuzzlesPerRow) )
		curPuzzlesPerRow *= 2;

	puzzlesize = sizeold / curPuzzlesPerRow;

	Assert( gProcShaderCTRL );
	if ( gProcShaderCTRL )
	{
		float deltapixelsize = ( curPuzzlesPerRow / (float)sizeold ) * 0.5f;
		gProcShaderCTRL->SetNormalizedPuzzleDelta( deltapixelsize );
	}
}

const int &GetCurrentPuzzleIndex()
{
	return curPuzzleIndex;
}
void OnPuzzleDrawn()
{
	curPuzzleIndex++;
}
void GetUVsForPuzzle( const int &num, Vector2D &min, Vector2D &max, bool bAddClamp )
{
	Assert( num <= (curPuzzlesPerRow*curPuzzlesPerRow) );

	double puzzleUVSize = 1.0f / curPuzzlesPerRow;
	int row = num / curPuzzlesPerRow;
	int column = num % curPuzzlesPerRow;

	min.Init( column * puzzleUVSize, row * puzzleUVSize );
	max = min;
	max.x += puzzleUVSize;
	max.y += puzzleUVSize;

	if ( bAddClamp )
	{
		float deltapixelsize = 0.75f / (float)sizeold;
		min.x += deltapixelsize;
		min.y += deltapixelsize;
		max.x -= deltapixelsize;
		max.y -= deltapixelsize;
	}
	else
	{
		float deltapixelsize = sedit_uv_adjust.GetFloat() / (float)sizeold; // lé magic
		//float deltapixelsize = ( curPuzzlesPerRow / (float)sizeold ) * 0.1f;
		//min.x += deltapixelsize;
		//min.y += deltapixelsize;
		max.x += deltapixelsize;
		max.y += deltapixelsize;
	}
}
void GetCurPuzzleView( int &x, int &y, int &w, int &t )
{
	Assert( curPuzzleIndex <= (curPuzzlesPerRow*curPuzzlesPerRow) );

	int puzzlePixelSize = sizeold / curPuzzlesPerRow;
	int row = curPuzzleIndex / curPuzzlesPerRow;
	int column = curPuzzleIndex % curPuzzlesPerRow;

	x = column * puzzlePixelSize;
	y = row * puzzlePixelSize;
	w = t = puzzlePixelSize;
}

void CleanupString( const char *pSrc, char *pOut, int maxLength, bool AllowSpaces, bool AllowUnderscore, bool AllowNumbers, bool AllowNumberInFirstPlace )
{
	Assert( maxLength > 1 );
	int slotWrite = 0;
	const char *walk = pSrc;

	while (walk && *walk && slotWrite < (maxLength-1))
	{
		if ( ( *walk >= 'a' && *walk <= 'z' ) ||
			( *walk >= 'A' && *walk <= 'Z' ) ||
			( AllowNumbers && *walk >= '0' && *walk <= '9' ) ||
			( AllowUnderscore && *walk == '_' ) ||
			( *walk == ' ' ) )
		{
			pOut[slotWrite] = *walk;
			if ( !AllowSpaces && *walk == ' ' )
				pOut[slotWrite] = '_';
			else if (!AllowNumberInFirstPlace && !slotWrite && *walk >= '0' && *walk <= '9')
				pOut[slotWrite] = '_';
			slotWrite++;
		}
		walk++;
	}

	pOut[slotWrite] = '\0';
}

void ForceDeleteMaterial( IMaterial **pMat )
{
	if ( !GetMatSysShutdownHelper()->IsShuttingDown() )
	{
#if 0
		const char *pszName = (*pMat)->GetName();
		const char *pszTexGroup = (*pMat)->GetTextureGroupName();

		char tmp[MAX_PATH];
		char tmpTexGroup[MAX_PATH];

		Q_snprintf( tmpTexGroup, sizeof(tmpTexGroup), "%s", pszTexGroup ? pszTexGroup : TEXTURE_GROUP_OTHER );

		if ( pszName )
		{
			Q_snprintf( tmp, sizeof(tmp), "%s", pszName );

			while ( !IsErrorMaterial( materials->FindProceduralMaterial( tmp, tmpTexGroup, NULL ) ) )
			{
				(*pMat)->Release();
				(*pMat)->DeleteIfUnreferenced();
			}
		}
		else
#endif
		{
			//Assert( 0 );

			// yuck. shoot me now.
			for (int i = 0; i < 99; i++ )
#ifdef SHADER_EDITOR_DLL_2006
				(*pMat)->DecrementReferenceCount();
#else
				(*pMat)->Release();
#endif

			(*pMat)->DeleteIfUnreferenced();
		}
	}

	(*pMat) = NULL;
}
// same shit again.
void ForceDeleteTexture( ITexture **pTex )
{
	if ( !GetMatSysShutdownHelper()->IsShuttingDown() )
	{
		const char *pszName = (*pTex)->GetName();

		char tmp[MAX_PATH];
		if ( pszName )
		{
			Q_snprintf( tmp, sizeof(tmp), "%s", pszName );

			while ( !IsErrorTexture( materials->FindTexture( tmp, TEXTURE_GROUP_OTHER, false ) ) )
			{
#ifdef SHADER_EDITOR_DLL_2006
				(*pTex)->DecrementReferenceCount();
#else
				(*pTex)->Release();
#endif
				(*pTex)->DeleteIfUnreferenced();
			}
		}
		else
		{
			Assert( 0 );

#ifdef SHADER_EDITOR_DLL_2006
			(*pTex)->DecrementReferenceCount();
#else
			(*pTex)->Release();
#endif
			(*pTex)->DeleteIfUnreferenced();
		}
	}

	(*pTex) = NULL;
}

IMaterialVar *FindMaterialVar( IMaterial *pMat, const char *varName )
{
	bool bSuccess = false;
	IMaterialVar *pVar = pMat->FindVar( varName, &bSuccess );
	return bSuccess ? pVar : NULL;
}
bool SetMaterialVar_String( IMaterial *pMat, const char *varName, const char *value )
{
	IMaterialVar *pVar = FindMaterialVar( pMat, varName );
	if ( !pVar )
		return false;
	pVar->SetStringValue( value );
	return true;
}
bool SetMaterialVar_Vec( IMaterial *pMat, const char *varName, const float a, const float b, const float c, const float d )
{
	IMaterialVar *pVar = FindMaterialVar( pMat, varName );
	if ( !pVar )
		return false;
	pVar->SetVecValue( a, b, c, d );
	return true;
}
bool SetMaterialVar_Vec( IMaterial *pMat, const char *varName, const float a, const float b, const float c )
{
	IMaterialVar *pVar = FindMaterialVar( pMat, varName );
	if ( !pVar )
		return false;
	pVar->SetVecValue( a, b, c );
	return true;
}
bool SetMaterialVar_Vec( IMaterial *pMat, const char *varName, const float a, const float b )
{
	IMaterialVar *pVar = FindMaterialVar( pMat, varName );
	if ( !pVar )
		return false;
	pVar->SetVecValue( a, b );
	return true;
}
bool SetMaterialVar_Vec( IMaterial *pMat, const char *varName, const float a )
{
	IMaterialVar *pVar = FindMaterialVar( pMat, varName );
	if ( !pVar )
		return false;
	pVar->SetFloatValue( a );
	return true;
}
bool SetMaterialVar_Int( IMaterial *pMat, const char *varName, const int a )
{
	IMaterialVar *pVar = FindMaterialVar( pMat, varName );
	if ( !pVar )
		return false;
	pVar->SetIntValue( a );
	return true;
}

void BuildUserFunctionName( const char *pFunctionName, const CUtlVector< __funcParamSetup* > &sharedOutputs, char *pszDest, int buflen )
{
	if ( buflen )
		pszDest[0] = '\0';

	if ( buflen < 6 )
		return;

	Q_snprintf( pszDest, buflen, "void %s(", pFunctionName );

	char *pWalk = pszDest + 5;
	while ( pWalk && *pWalk )
	{
		if ( *pWalk == ' ' )
			*pWalk = '_';
		pWalk++;
	}

#if DEBUG
	bool bHitOutput = false;
#endif

	for ( int i = 0; i < sharedOutputs.Count(); i++ )
	{
		__funcParamSetup *pData = sharedOutputs[i];

#if DEBUG
		bHitOutput = bHitOutput || pData->bOutput;
		Assert( pData->bOutput == bHitOutput );
#endif

		Q_strcat( pszDest, pData->bOutput ? " out " : " in ", buflen );
		Q_strcat( pszDest, GetVarCodeNameFromFlag( pData->iFlag ), buflen );
		Q_strcat( pszDest, " ", buflen );
		Q_strcat( pszDest, pData->GetSafeName( i ), buflen );

		if ( i < sharedOutputs.Count() - 1 )
			Q_strcat( pszDest, ",", buflen );
		else
			Q_strcat( pszDest, " ", buflen );
	}

	Q_strcat( pszDest, ")", buflen );
}

void BuildUserFunctionName( const char *pFunctionName, const CUtlVector< __funcParamSetup* > &input, const CUtlVector< __funcParamSetup* > &output,
							char *pszDest, int buflen )
{
	CUtlVector< __funcParamSetup* >hOutputs;
	for ( int i = 0; i < input.Count(); i++ )
		hOutputs.AddToTail( new __funcParamSetup( *input[i] ) );
	for ( int i = 0; i < output.Count(); i++ )
		hOutputs.AddToTail( new __funcParamSetup( *output[i] ) );

	BuildUserFunctionName( pFunctionName, hOutputs, pszDest, buflen );

	hOutputs.PurgeAndDeleteElements();
}

void BuildDefaultFunctionName( char *pszDest, int buflen, CBaseNode *pNode )
{
	Q_snprintf( pszDest, buflen, "UserFunction_%u", pNode->GetUniqueIndex() );
}