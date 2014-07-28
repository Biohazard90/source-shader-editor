
#include "procshader/shaderincludes.h"

#include "ProcShaderInterface.h"

#include "tier1.h"

#define VENGINE_CLIENT_RANDOM_INTERFACE_VERSION	"VEngineRandom001"

static ProcShaderInterface __g_ProcShaderCTRL;
ProcShaderInterface *gProcShaderCTRL = &__g_ProcShaderCTRL;

BasicShaderCfg_t *pShaderCFG[2] = { NULL, NULL };

#ifndef SHADER_EDITOR_DLL_SWARM
IFileSystem *g_pFullFileSystem = NULL;
#else
extern IFileSystem *g_pFullFileSystem;
#endif
IUniformRandomStream *random = NULL;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( ProcShaderInterface, IVProcShader, PROCSHADER_INTERFACE_VERSION, __g_ProcShaderCTRL );

ProcShaderInterface::ProcShaderInterface()
{
	_pixeldelta = 0;
	hCallbackList = NULL;
}
ProcShaderInterface::~ProcShaderInterface()
{
	Assert( !hPreloadList.Count() ); // must be destructed in editordll heap
}

//extern CreateInterfaceFn baseFactory;

IVPPEHelper *gPPEHelper = NULL;

bool ProcShaderInterface::Init( CreateInterfaceFn appSystemFactory, IVPPEHelper *pPPEHelper )
{
	gPPEHelper = pPPEHelper;

	if ( (g_pFullFileSystem = (IFileSystem *)appSystemFactory(FILESYSTEM_INTERFACE_VERSION, NULL)) == NULL )
		return false;

	if ( (random = (IUniformRandomStream *)appSystemFactory(VENGINE_CLIENT_RANDOM_INTERFACE_VERSION, NULL)) == NULL )
		return false;

	return true;
}
void ProcShaderInterface::Shutdown()
{
}

void ProcShaderInterface::LoadInterfacesOnDemand()
{
}

void *ProcShaderInterface::SwapShaderSystem( void *_data, const int &index )
{
	Assert( index >= 0 && index < 2 );
	m_Lock.Lock();
	BasicShaderCfg_t *tmp = pShaderCFG[index];
	pShaderCFG[index] = (BasicShaderCfg_t*)_data;
	m_Lock.Unlock();
	return tmp;
}

BasicShaderCfg_t *ProcShaderInterface::AccessVolatileData( const int index )
{
	Assert( index >= 0 && index < 2 );
	m_Lock.Lock();
	m_Lock.Unlock();
	return pShaderCFG[index];
}

void ProcShaderInterface::SetNormalizedPuzzleDelta( float d )
{
	_pixeldelta = d;
}

static float _envdataCollection[HLSLENV_MAX][4];
void ProcShaderInterface::UpdateEnvironmentData( int iEnvC, float *_fl4 )
{
	Q_memcpy( _envdataCollection[iEnvC], _fl4, sizeof(float) * 4 );
}

float *ProcShaderInterface::AccessEnvConstant( int i )
{
	return _envdataCollection[i];
}


void ProcShaderInterface::AddPreloadShader( void *data )
{
	m_Lock.Lock();

	BasicShaderCfg_t *pShader = (BasicShaderCfg_t*)data;
	hPreloadList.AddToTail( pShader );

	m_Lock.Unlock();
}
int ProcShaderInterface::GetNumPreloadShaders()
{
	m_Lock.Lock();

	int num = hPreloadList.Count();

	m_Lock.Unlock();
	return num;
}
int ProcShaderInterface::FindPreloadShader( const char *name )
{
	m_Lock.Lock();
	int index = -1;
	for ( int i = 0; i < hPreloadList.Count(); i++ )
	{
		Assert( hPreloadList[i]->CanvasName );
		if ( !Q_stricmp( hPreloadList[i]->CanvasName, name ) )
		{
			index = i;
		}
	}
	m_Lock.Unlock();
	return index;
}
void *ProcShaderInterface::GetPreloadShader( const int idx )
{
	m_Lock.Lock();

	Assert( hPreloadList.IsValidIndex( idx ) );
	BasicShaderCfg_t *pOut = hPreloadList[idx];

	m_Lock.Unlock();
	return pOut;
}
void *ProcShaderInterface::SwapPreloadShader( const int idx, void *_data )
{
	m_Lock.Lock();

	Assert( hPreloadList.IsValidIndex( idx ) );
	BasicShaderCfg_t *pOut = hPreloadList[idx];
	hPreloadList[idx] = (BasicShaderCfg_t*)_data;

	m_Lock.Unlock();
	return pOut;
}
void *ProcShaderInterface::GetAndRemovePreloadShader( const int idx )
{
	m_Lock.Lock();

	Assert( hPreloadList.IsValidIndex( idx ) );
	BasicShaderCfg_t *pOut = hPreloadList[idx];
	hPreloadList.Remove(idx);

	m_Lock.Unlock();
	return pOut;
}

BasicShaderCfg_t *ProcShaderInterface::GetPreloadShader_Internal( const int &idx )
{
	Assert( hPreloadList.IsValidIndex( idx ) );
	return hPreloadList[ idx ];
}
BasicShaderCfg_t *ProcShaderInterface::GetPreloadShader_Internal( const char *name, int *index )
{
	for ( int i = 0; i < hPreloadList.Count(); i++ )
	{
		Assert( hPreloadList[i]->CanvasName );
		if ( !Q_stricmp( hPreloadList[i]->CanvasName, name ) )
		{
			if ( index )
				*index = i;
			return hPreloadList[i];
		}
	}
	return NULL;
}

void ProcShaderInterface::LinkCallbacks( CUtlVector< _clCallback* > *hList )
{
	hCallbackList = hList;
}
int ProcShaderInterface::GetNumCallbacks()
{
	if ( !hCallbackList )
		return 0;
	return hCallbackList->Count();
}
_clCallback *ProcShaderInterface::GetCallback( int i )
{
	Assert( hCallbackList );
	Assert( hCallbackList->IsValidIndex( i ) );
	return hCallbackList->Element( i );
}
int ProcShaderInterface::FindCallback( const char *name )
{
	if ( !name || !Q_strlen( name ) )
		return -1;
	Assert( hCallbackList );
	for ( int i = 0; i < hCallbackList->Count(); i++ )
	{
		if ( !Q_stricmp( hCallbackList->Element( i )->name, name ) )
			return i;
	}
	return -1;
}
