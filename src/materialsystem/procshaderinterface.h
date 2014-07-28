#ifndef PROCSHADER_INTERFACE_H
#define PROCSHADER_INTERFACE_H

#include "IVProcShader.h"
#include "filesystem.h"
#include "vstdlib/random.h"
#include "tier1/KeyValues.h"

#define STANDARDTEX_USER_FIRST_INDEX TEXTURE_BLACK //( TEXTURE_IDENTITY_LIGHTWARP + 1 )

class ProcShaderInterface : public IVProcShader
{
public:
	ProcShaderInterface();
	~ProcShaderInterface();

	virtual bool Init( CreateInterfaceFn appSystemFactory, IVPPEHelper *pPPEHelper );
	virtual void Shutdown();
	
	virtual void *SwapShaderSystem( void *_data, const int &index );
	virtual void SetNormalizedPuzzleDelta( float d );
	virtual void UpdateEnvironmentData( int iEnvC, float *_fl4 );

	BasicShaderCfg_t *AccessVolatileData( const int index = 0 );
	float *AccessEnvConstant( int i );
	float &GetNormalizedPuzzleDelta(){return _pixeldelta;};

	virtual void AddPreloadShader( void *data );
	virtual int GetNumPreloadShaders();

	virtual int FindPreloadShader( const char *name );
	virtual void *GetPreloadShader( const int idx );
	virtual void *GetAndRemovePreloadShader( const int idx );
	virtual void *SwapPreloadShader( const int idx, void *_data );

	virtual BasicShaderCfg_t *GetPreloadShader_Internal( const int &idx );
	virtual BasicShaderCfg_t *GetPreloadShader_Internal( const char *name, int *index = NULL );

	virtual void LinkCallbacks( CUtlVector< _clCallback* > *hList );
	int GetNumCallbacks();
	_clCallback *GetCallback( int i );
	int FindCallback( const char *name );

	void LoadInterfacesOnDemand();

private:
	CThreadMutex m_Lock;

	float _pixeldelta;

	CUtlVector< BasicShaderCfg_t* >hPreloadList;
	CUtlVector< _clCallback* >*hCallbackList;
};

class CProceduralContext;

bool IsTextypeUsingCustomTexture( int textype );
void BindTextureByAutoType( bool bPreview, IShaderDynamicAPI *pShaderAPI, CBaseVSShader *pShader,
							int type, int sampleridx, int TextureVar, int FrameVar = -1, bool bPS = true );
void UpdateConstantByIdentifier( CBaseVSShader *pShader, IShaderDynamicAPI* pShaderAPI, IMaterialVar **params, SimpleEnvConstant *pConst, CProceduralContext *pContext,
								bool bPS, int iFirstMutable = -1, int iFirstStatic = -1 );

extern IUniformRandomStream *random;
extern IFileSystem *g_pFullFileSystem;

#endif