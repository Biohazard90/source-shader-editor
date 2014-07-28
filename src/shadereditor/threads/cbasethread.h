#ifndef CBASE_THREAD_H
#define CBASE_THREAD_H

#include "cbase.h"
#include "editorCommon.h"

class CBaseThread : public CThread
{
public:
	CBaseThread();
	CMessageQueue< __threadcmds_common* > m_Queue_Common;

	int CheckCommon();

	bool ShouldDie();
	int GetRefCount();
	void IncrementRefCount();
	void DecrementRefCount();

private:
	int _RefCount;
	CThreadMutex mutex_refcount;
};


#endif