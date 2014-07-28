
#include "cbase.h"
#include "editorCommon.h"


CThreadManager __gThreadCtrl( "thread_controller" );
CThreadManager *gThreadCtrl = &__gThreadCtrl;

CThreadManager::CThreadManager( char const *name ) : CAutoGameSystemPerFrame( name )
{
}

CThreadManager::~CThreadManager()
{
}

void CThreadManager::Update( float frametime )
{
	if ( m_QueueCompileCallback.MessageWaiting() )
	{
		__threadcmds_CompileCallback *msg;
		m_QueueCompileCallback.WaitMessage( &msg );

		CAutoSolverCallback *callback = CAutoSolverCallback::GetSolverClientByIdx( msg->_Target );
		if (callback)
			callback->OnCompilationEnd( msg->_activeFlags, msg->_ExitCode, msg->_data );

		if ( msg->_ExitCode == 0 &&
			msg->_data != NULL &&
			!msg->_data->IsPreview() )
		{
			CAutoSolverCallback::CopyShaders( msg->_activeFlags, msg->_data );

			df_SaveDump_File( msg->_data->name, *msg->_data->shader );

			if ( msg->_activeFlags & ACTIVEFLAG_VS && msg->_activeFlags & ACTIVEFLAG_PS )
				pEditorRoot->SendFullyCompiledShader( msg->_data );
		}

		//delete msg->_data;
		//msg->_data = NULL;
		delete msg;
	}
}

void CThreadManager::Shutdown()
{
	m_hCompileThread.Detach();
	for ( int i = 0; i < m_hThreads.Count(); i++ )
	{
		CBaseThread *t = m_hThreads[ i ];
		t->Stop();
		t->Terminate();
		//while ( t->IsAlive() ) {}
		//delete t;
	}
	m_hThreads.Purge();
}

CCompileThread *CThreadManager::CreateCompileThread()
{
	CCompileThread *thread = CCompileThread::CreateWriteFXCThread();
	m_hThreads.AddToTail(thread);
	return thread;
}
CCompileThread *CThreadManager::GetCompileThread()
{
	if ( !m_hCompileThread.IsValid() )
		m_hCompileThread.Attach( CreateCompileThread() );

	m_hCompileThread->IncrementRefCount();
	return m_hCompileThread.Get();
}
void CThreadManager::StopThread( CBaseThread *t, bool bTerminate )
{
	if ( !t )
		return;

	t->DecrementRefCount();
	if ( !t->ShouldDie() )
		return;

	if ( !t->IsAlive() )
		return;

	if ( !bTerminate )
	{
		__threadcmds_common *com = new __threadcmds_common();
		com->_cmd = TCMD_COMMON_EXIT;
		t->m_Queue_Common.QueueMessage( com );
		m_hThreads.FindAndRemove( t );
	}
	else
	{
		m_hThreads.FindAndRemove( t );
		t->Terminate();
		//t->Stop();
		//delete t;
	}
}