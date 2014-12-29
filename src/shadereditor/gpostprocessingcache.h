#ifndef POST_PROCESSING_CACHE_H
#define POST_PROCESSING_CACHE_H

#include "cbase.h"
#include "editorCommon.h"

// used to actually call the func
struct EditorRenderViewCommand_Data
{
public:

	EditorRenderViewCommand_Data();
	~EditorRenderViewCommand_Data();
	EditorRenderViewCommand_Data( const EditorRenderViewCommand_Data &o );

	void AddBoolValue( bool bVal );
	void AddIntValue( int iVal );
	void AddFloatValue( float flVal );
	void AddStringValue( const char *pszVal );

	const bool GetBoolVal( const int &slot );
	const int GetIntVal( const int &slot );
	const float GetFloatVal( const int &slot );
	const char *GetStringVal( const int &slot );

	const int GetNumBool(){ return hValues_Bool.Count(); };
	const int GetNumInt(){ return hValues_Int.Count(); };
	const int GetNumFloat(){ return hValues_Float.Count(); };
	const int GetNumString(){ return hValues_String.Count(); };

	void ClearAllValues();
	void ValidateMemory();
	void CallFunction();

	const char *GetName();
	void SetName( const char *name );

private:
	CUtlVector< bool > hValues_Bool;
	CUtlVector< int > hValues_Int;
	CUtlVector< float > hValues_Float;
	CUtlVector< char* > hValues_String;

	char *m_pszCallbackName;

	pFnVrCallback( functor );
};

// instantiated through the client
struct EditorRenderViewCommand_Definition
{
public:

	EditorRenderViewCommand_Definition();
	~EditorRenderViewCommand_Definition();

	enum
	{
		VAR_BOOL = 0,
		VAR_INT,
		VAR_FLOAT,
		VAR_STRING,
		VAR_AMT,
	};

	const char *GetName();
	void SetName( const char *name );

	void AddVarToList( int type, const char *szName );
	const char *GetVarName( int type, int slot );
	const int GetNumVars( int type );

	pFnVrCallback( functor );

	EditorRenderViewCommand_Data defaults;

private:
	EditorRenderViewCommand_Definition( const EditorRenderViewCommand_Definition &o );

	CUtlVector< CUtlVector< char* >* >hVar_Names;
	char *m_pszCallbackName;
};


struct EditorPostProcessingEffect
{
public:

	EditorPostProcessingEffect();
	EditorPostProcessingEffect( CUtlVector< CHLSL_SolverBase* > &hSolvers, GenericPPEData &config, bool bCopySolvers = true );
	~EditorPostProcessingEffect();
	EditorPostProcessingEffect( const EditorPostProcessingEffect &o );

	void Init();

	char *pszName;
	char *pszPath;

	bool bStartEnabled;
	bool bReady;
	bool bIsEnabled;
	bool bOwnsSolvers;

	int LoadPostProcessingChain( const char *Path, bool bStartEnabled );
	void ReplacePostProcessingChain( CUtlVector< CHLSL_SolverBase* > &hNewSolvers );
	IMaterial *FindMaterial( const char *pszNodeName );

	CUtlVector< CHLSL_SolverBase* > hSolverArray;
	GenericPPEData config;
};


class CPostProcessingCache : public CAutoGameSystem
{
public:

	CPostProcessingCache();
	~CPostProcessingCache();

	enum PPE_Error
	{
		PPE_OKAY = 0,
		PPE_INVALID_INPUT,
		PPE_INVALID_GRAPH_TYPE,
		PPE_GRAPH_NOT_VALID,
	};

	virtual bool Init();
	virtual void Shutdown();

	const char *GetPPEffectPrecacheFile();
	void LoadPrecacheFile();
	void SavePrecacheFile();
	void ClearInvalidEntries();

	void RefreshAllPPEMaterials();
	void RefreshAllPPEMaterials( CUtlVector< CHLSL_SolverBase* > &hSolvers );

	int GetNumPostProcessingEffects();
	EditorPostProcessingEffect *GetPostProcessingEffect( int num );
	int FindPostProcessingEffect( const char *name );
	PPE_Error AddPostProcessingEffect( EditorPostProcessingEffect *effect );
	PPE_Error LoadPostProcessingEffect( const char *path, bool bStartEnabled );
	void DeletePostProcessingEffect( int index );

	void SortPostProcessingEffects();
	void MoveEffectAlongList( int index, bool bUp );

	void RenderAllEffects( bool bScene );
	static void RenderSinglePPE( EditorPostProcessingEffect *effect, bool bPreviewMode = false, bool bSceneMode = false, bool bOwnsEffect = false );
	static void RenderSinglePPE( EditorPostProcessingEffect *effect, int x, int y, int w, int h,
		bool bPreviewMode = false, bool bSceneMode = false, bool bOwnsEffect = false );

	void LockVrCallbacks();
	bool AddVrCallback( EditorRenderViewCommand_Definition *pCallback );
	int GetNumVrCallbacks();
	EditorRenderViewCommand_Definition *GetVrCallback( int index );
	int FindVrCallback( const char *szName );

private:

	CUtlVector< EditorPostProcessingEffect >hEffects;
	CUtlVector< EditorRenderViewCommand_Definition* > m_hVrCallbacks;

	bool m_bVrCallbacksLocked;

	void InitializeMaterials();

};


CPostProcessingCache *GetPPCache();

#endif