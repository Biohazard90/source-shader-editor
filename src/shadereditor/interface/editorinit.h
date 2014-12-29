#ifndef EDITORINIT_H
#define EDITORINIT_H

class ShaderEditorInterface;

#include "IVProcShader.h"
#include "IVShaderEditor.h"


class ShaderEditorInterface : public IVShaderEditor
{
public:
	ShaderEditorInterface();
	~ShaderEditorInterface();

	bool Init( CreateInterfaceFn appSystemFactory, CGlobalVarsBase *pGlobals,
		void *pSEditMRender,
		bool bCreateEditor, bool bEnablePrimaryDebug, int iSkymaskMode );
	void Shutdown();
	void PrecacheData();

	void OverridePaths( const char *pszWorkingDirectory, const char *pszCompilePath, const char *pszLocalCompilePath,
		const char *pszGamePath, const char *pszCanvas, const char *pszShaderSource, const char *pszDumps,
		const char *pszUserFunctions, const char *pszEditorRoot );

	void OnFrame( float frametime );
	void OnPreRender( void *viewsetup );
	void OnSceneRender();
	void OnUpdateSkymask( bool bCombineMode, int x, int y, int w, int h );
	void OnPostRender( bool bUpdateFB );

	void RegisterClientCallback( const char *name, pFnClCallback(callback), int numComponents );
	void LockClientCallbacks();

	int GetNumCallbacks();
	_clCallback *GetCallback( int idx );
	int FindCallback( const char *name );

	void RegisterViewRenderCallback( const char *pszVrCName, pFnVrCallback(callback),
		const char **pszBoolNames, const bool *pBoolDefaults, const int numBoolParams,
		const char **pszIntNames, const int *pIntDefaults, const int numIntParams,
		const char **pszFloatNames, const float *pFloatDefaults, const int numFloatParams,
		const char **pszStringNames, const char **pStringDefaults, const int numStringParams );
	void LockViewRenderCallbacks();

	int			GetPPEIndex( const char *pszName );
	bool		IsPPEEnabled( const int &index );
	void		SetPPEEnabled( const int &index, const bool &bEnabled );
	IMaterial	*GetPPEMaterial( const int &index, const char *pszNodeName );
	void		DrawPPEOnDemand( const int &index, const bool bInScene );
	void		DrawPPEOnDemand( const int &index, int x, int y, int w, int h, const bool bInScene );
	IMaterialVar	*GetPPEMaterialVarFast( ShaderEditVarToken &token,
		const char *pszPPEName, const char *pszNodeName, const char *pszVarName );

	const bool ShouldShowPrimaryDbg(){ return m_bShowPrimaryDebug; };

	void SetFramebufferCopyTexOverride( ITexture *tex );
	void UpdateFramebufferTexture( bool bDoPush = false, bool bCopyToEditorTarget = false );

	void FlushPPEMaterialVarCache();

private:
	static bool bCallbackLock;
	static bool bVrCallbackLock;
	bool m_bShowPrimaryDebug;

	CUtlVector< _clCallback* > m_hFunctionCallbacks;

	KeyValues *pSkyMats[3];

	unsigned int m_iLastInvalidVarToken;
	unsigned int m_iNextValidVarToken;
	CUtlVector< IMaterialVar* > m_cachedMVars;
};


class MainShaderEditorUpdate : public CAutoGameSystemPerFrame
{
public:
	MainShaderEditorUpdate( char const *name );
	~MainShaderEditorUpdate();

	virtual bool Init();
	virtual void Update( float frametime );

	void QueueLog( const char *_data, int len );
	void QueueLog( const char *_data );
	void ParseLog( char *logOut, int maxLen );
	bool IsLogDirty();

	void PreRender();
	void UpdateConstants( const CViewSetup *_override );
	void UpdateConstants( const CViewSetup_SEdit_Shared *_override = NULL );

	void SetCompilerState( bool bRunning );
	bool IsCompilerRunning();

	void BeginMaterialReload( bool bResetMode );

private:
	CThreadMutex m_BufferLock;
	CUtlBuffer m_buf_Log;
	bool bDirty;

	CThreadMutex m_LockCompilerState;
	bool bCompilerRunning;
};

extern void ForceTerminateCompilers();

extern ShaderEditorInterface *shaderEdit;
extern MainShaderEditorUpdate *gShaderEditorSystem;

void CheckDirectories();

#endif