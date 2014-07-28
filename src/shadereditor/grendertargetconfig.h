#ifndef RTCONFIG_H
#define RTCONFIG_H

#include "cbase.h"
#include "editorCommon.h"

#define RT_DEFAULT_SIZE 256
#define RT_MAX_SIZE 4096

#ifdef SHADER_EDITOR_DLL_2013
#define LAST_VALVE_RT_SIZEMODE RT_SIZE_REPLAY_SCREENSHOT
#else
#define LAST_VALVE_RT_SIZEMODE RT_SIZE_FULL_FRAME_BUFFER_ROUNDED_UP
#endif

#define RT_SIZE_HALF_FRAME_BUFFER ((RenderTargetSizeMode_t)(LAST_VALVE_RT_SIZEMODE+1))

class CMaterialSysDeletionHelper : public CAutoGameSystem
{
public:
	CMaterialSysDeletionHelper() : CAutoGameSystem( "matsysdeletionhelper" ){
		m_bShuttingDown = false;
	};

	void Shutdown(){
		m_bShuttingDown = true;
	};

	bool IsShuttingDown(){ return m_bShuttingDown; };

private:
	bool m_bShuttingDown;
};


class RenderTargetManager : public CAutoGameSystemPerFrame
{
public:

	RenderTargetManager();
	~RenderTargetManager();

	bool Init();
	void Shutdown();
	void PreRender();

	void MakeRTCacheDirty();
	void UpdateRTsToMatSys( bool bOnStartup = false );
	void SaveRTsToFile();
	const char *GetRTSetupFilePath();

	int GetNumRTs();
	RTDef *GetRT( int id );
	void RemoveRT( int index );
	void AddRT( RTDef rt );
	RTDef *FindRTByName( const char *name, bool bIgnoreFlaggedForDel = false );
	int FindRTIndexByName( const char *name, bool bIgnoreFlaggedForDel = false );

	void SortRTsByName();

private:
	CUtlVector< RTDef > m_hRTs;
	bool m_bCacheDirty;
};

RenderTargetManager *GetRTManager();
CMaterialSysDeletionHelper *GetMatSysShutdownHelper();


#endif