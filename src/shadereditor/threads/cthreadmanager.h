#ifndef CTHREADMANAGER_H
#define CTHREADMANAGER_H

#include "editorCommon.h"

class CThreadManager : public CAutoGameSystemPerFrame
{
public:
	CThreadManager( char const *name );
	~CThreadManager();

	virtual void Update( float frametime );
	virtual void Shutdown();
	
	//CSolveThread *CreateSolverThread();
	CCompileThread *GetCompileThread();
	void StopThread( CBaseThread *t, bool bTerminate = false );

	CMessageQueue< __threadcmds_CompileCallback* > m_QueueCompileCallback;
	//CMessageQueue< __threadcmds_RenderCallback* > m_QueueRenderCallback;

private:
	CUtlVector< CBaseThread* > m_hThreads;
	//CUtlVector< CBaseThread* > m_hThreads_CleanupPile;

	CCompileThread *CreateCompileThread();
	CPlainAutoPtr< CCompileThread > m_hCompileThread;
};

extern CThreadManager *gThreadCtrl;

#endif