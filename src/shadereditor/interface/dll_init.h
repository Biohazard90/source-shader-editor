#ifndef DLL_INIT_H
#define DLL_INIT_H
#ifdef _WIN32
#pragma once
#endif

//#include "iclientnetworkable.h"
#include "utllinkedlist.h"
#include "cdll_int.h"
#include "eiface.h"
#include <soundflags.h>
#ifdef SHADER_EDITOR_DLL_2006
#include "vector.h"
#include "datacache/imdlcache.h"
#else
#include "mathlib/vector.h"
#endif
#include <shareddefs.h>
#include "materialsystem/materialsystemutil.h"
#include "cmodel.h"
#include "utlvector.h"
#include "engine/IEngineTrace.h"
#include "engine/IStaticPropMgr.h"
#include "networkvar.h"

class IVModelRender;
class IVEngineClient;
class IVModelRender;
class IVRenderView;
class IMaterialSystem;
class IDataCache;
class IMDLCache;
class IVModelInfoClient;
class IEngineVGui;
class IFileSystem;
class IUniformRandomStream;
class CGaussianRandomStream;
class IEngineSound;
class IMatSystemSurface;
class IMaterialSystemHardwareConfig;
class IGameUIFuncs;
class CGlobalVarsBase;
class IInputSystem;
class CSteamAPIContext;

extern IVModelRender *modelrender;
extern IVEngineClient	*engine;
extern IVModelRender *modelrender;
extern IVRenderView *render;
extern IMaterialSystem *materials;
extern IMaterialSystemHardwareConfig *g_pMaterialSystemHardwareConfig;
extern IDataCache *datacache;
extern IMDLCache *mdlcache;
extern IVModelInfoClient *modelinfo;
extern IEngineVGui *enginevgui;
extern IFileSystem *filesystem;
extern IUniformRandomStream *random;
extern CGaussianRandomStream *randomgaussian;
extern IEngineSound *enginesound;
extern IMatSystemSurface *g_pMatSystemSurface;
extern IGameUIFuncs *gameuifuncs;
extern CGlobalVarsBase *gpGlobals;
extern IInputSystem *inputsystem;
extern CSteamAPIContext *steamapicontext;

char	*VarArgs( char *format, ... );
int		ScreenHeight( void );
int		ScreenWidth( void );









#endif