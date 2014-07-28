#include "cbase.h"
#include "editorCommon.h"


CBaseThread::CBaseThread()
{
	_RefCount = 0;
}
bool CBaseThread::ShouldDie()
{
	bool b;
	mutex_refcount.Lock();
	b = _RefCount <= 0;
	mutex_refcount.Unlock();
	return b;
}
int CBaseThread::GetRefCount()
{
	int tmp;
	mutex_refcount.Lock();
	tmp = _RefCount;
	mutex_refcount.Unlock();
	return tmp;
}
void CBaseThread::IncrementRefCount()
{
	mutex_refcount.Lock();
	_RefCount++;
	mutex_refcount.Unlock();
}
void CBaseThread::DecrementRefCount()
{
	mutex_refcount.Lock();
	_RefCount--;
	mutex_refcount.Unlock();
}
int CBaseThread::CheckCommon()
{
	if ( m_Queue_Common.MessageWaiting() )
	{
		__threadcmds_common *msg_common;
		m_Queue_Common.WaitMessage( &msg_common );
		switch ( msg_common->_cmd )
		{
		case TCMD_COMMON_EXIT:
			delete msg_common;
			return TCMD_COMMON_EXIT;
		}
		delete msg_common;
	}
	return 0;
}