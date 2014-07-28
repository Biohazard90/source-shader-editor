
#include "cbase.h"
#ifndef SHADER_EDITOR_DLL_2006
#include "tier1/callqueue.h"
#endif
#include "editorCommon.h"


static CPostProcessingCache gs_ppcache;
CPostProcessingCache *GetPPCache(){ return &gs_ppcache; };


EditorRenderViewCommand_Data::EditorRenderViewCommand_Data()
{
	AllocCheck_Alloc();

	m_pszCallbackName = NULL;
	functor = NULL;
}
EditorRenderViewCommand_Data::~EditorRenderViewCommand_Data()
{
	AllocCheck_Free();

	delete [] m_pszCallbackName;

	for ( int i = 0; i < hValues_String.Count(); i++ )
		delete [] hValues_String[i];
}
EditorRenderViewCommand_Data::EditorRenderViewCommand_Data( const EditorRenderViewCommand_Data &o )
{
	AllocCheck_Alloc();

	functor = o.functor;

	hValues_Bool.AddVectorToTail( o.hValues_Bool );
	hValues_Int.AddVectorToTail( o.hValues_Int );
	hValues_Float.AddVectorToTail( o.hValues_Float );

	for ( int i = 0; i < o.hValues_String.Count(); i++ )
	{
		int len = Q_strlen( o.hValues_String[i] ) + 1;
		char *pszCpy = new char[ len ];
		Q_strcpy( pszCpy, o.hValues_String[i] );
		hValues_String.AddToTail( pszCpy );
	}

	AutoCopyStringPtr( o.m_pszCallbackName, &m_pszCallbackName );
}
void EditorRenderViewCommand_Data::AddBoolValue( bool bVal )
{
	hValues_Bool.AddToTail( bVal );
}
void EditorRenderViewCommand_Data::AddIntValue( int iVal )
{
	hValues_Int.AddToTail( iVal );
}
void EditorRenderViewCommand_Data::AddFloatValue( float flVal )
{
	hValues_Float.AddToTail( flVal );
}
void EditorRenderViewCommand_Data::AddStringValue( const char *pszVal )
{
	const char *pszDef = "";
	if ( !pszVal )
		pszVal = pszDef;

	int len = Q_strlen( pszVal ) + 1;
	char *pStr = new char[ len ];

	Q_strcpy( pStr, pszVal );
	hValues_String.AddToTail( pStr );
}
const bool EditorRenderViewCommand_Data::GetBoolVal( const int &slot )
{
	return hValues_Bool[ slot ];
}
const int EditorRenderViewCommand_Data::GetIntVal( const int &slot )
{
	return hValues_Int[ slot ];
}
const float EditorRenderViewCommand_Data::GetFloatVal( const int &slot )
{
	return hValues_Float[ slot ];
}
const char *EditorRenderViewCommand_Data::GetStringVal( const int &slot )
{
	return hValues_String[ slot ];
}
void EditorRenderViewCommand_Data::ClearAllValues()
{
	for ( int i = 0; i < hValues_String.Count(); i++ )
		delete [] hValues_String[i];

	hValues_Bool.Purge();
	hValues_Int.Purge();
	hValues_Float.Purge();
	hValues_String.Purge();

	delete [] m_pszCallbackName;
	m_pszCallbackName = NULL;
}
const char *EditorRenderViewCommand_Data::GetName()
{
	return m_pszCallbackName;
}
void EditorRenderViewCommand_Data::SetName( const char *name )
{
	delete [] m_pszCallbackName;
	m_pszCallbackName = NULL;

	AutoCopyStringPtr( name, &m_pszCallbackName );

	if ( !m_pszCallbackName )
	{
		m_pszCallbackName = new char[1];
		*m_pszCallbackName = '\0';
	}
}
void EditorRenderViewCommand_Data::ValidateMemory()
{
	if ( !GetName() )
		return;

	int index = GetName() ? GetPPCache()->FindVrCallback( GetName() ) : -1;
	Assert( index >= 0 );

	if ( index < 0 )
		return;

	EditorRenderViewCommand_Definition *pDef = GetPPCache()->GetVrCallback( index );
	if ( !pDef )
		return;

	functor = pDef->functor;

	while ( GetNumBool() < pDef->GetNumVars( EditorRenderViewCommand_Definition::VAR_BOOL ) )
		AddBoolValue( false );
	while ( GetNumInt() < pDef->GetNumVars( EditorRenderViewCommand_Definition::VAR_INT ) )
		AddIntValue( 0 );
	while ( GetNumFloat() < pDef->GetNumVars( EditorRenderViewCommand_Definition::VAR_FLOAT ) )
		AddFloatValue( 0 );
	while ( GetNumString() < pDef->GetNumVars( EditorRenderViewCommand_Definition::VAR_STRING ) )
		AddStringValue( "" );
}
void EditorRenderViewCommand_Data::CallFunction()
{
	Assert( functor != NULL );

	if ( functor == NULL )
		return;

	functor( hValues_Bool.Base(), hValues_Int.Base(), hValues_Float.Base(), hValues_String.Base() );
}





EditorRenderViewCommand_Definition::EditorRenderViewCommand_Definition()
{
	AllocCheck_Alloc();

	m_pszCallbackName = NULL;

	for ( int i = 0; i < VAR_AMT; i++ )
		hVar_Names.AddToTail( new CUtlVector< char* >() );
}
EditorRenderViewCommand_Definition::~EditorRenderViewCommand_Definition()
{
	AllocCheck_Free();

	for ( int i = 0; i < VAR_AMT; i++ )
	{
		for ( int s = 0; s < hVar_Names[i]->Count(); s++ )
			delete [] hVar_Names[i]->Element( s );
	}

	hVar_Names.PurgeAndDeleteElements();

	delete [] m_pszCallbackName;
}
void EditorRenderViewCommand_Definition::AddVarToList( int type, const char *szName )
{
	Assert( type >= 0 && type < VAR_AMT );

	int len = Q_strlen( szName ) + 1;
	char *szCopiedName = new char[ len ];
	Q_strcpy( szCopiedName, szName );

	hVar_Names[ type ]->AddToTail( szCopiedName );
}
const char *EditorRenderViewCommand_Definition::GetVarName( int type, int slot )
{
	Assert( type >= 0 && type < VAR_AMT );
	Assert( slot >= 0 && slot < hVar_Names[ type ]->Count() );

	return hVar_Names[ type ]->Element( slot );
}
const int EditorRenderViewCommand_Definition::GetNumVars( int type )
{
	Assert( type >= 0 && type < VAR_AMT );
	return hVar_Names[ type ]->Count();
}
const char *EditorRenderViewCommand_Definition::GetName()
{
	return m_pszCallbackName;
}
void EditorRenderViewCommand_Definition::SetName( const char *name )
{
	delete [] m_pszCallbackName;
	m_pszCallbackName = NULL;

	AutoCopyStringPtr( name, &m_pszCallbackName );

	if ( !m_pszCallbackName )
	{
		m_pszCallbackName = new char[1];
		*m_pszCallbackName = '\0';
	}
}





EditorPostProcessingEffect::EditorPostProcessingEffect()
{
	AllocCheck_Alloc();

	Init();
}
EditorPostProcessingEffect::EditorPostProcessingEffect( CUtlVector< CHLSL_SolverBase* > &hSolvers, GenericPPEData &config, bool bCopySolvers )
{
	AllocCheck_Alloc();

	Init();

	bOwnsSolvers = bCopySolvers;

	this->config = config;
	if ( bCopySolvers )
		CopySolvers( hSolvers, hSolverArray );
	else
		hSolverArray.AddVectorToTail( hSolvers );

	bReady = true;
}
void EditorPostProcessingEffect::Init()
{
	pszName = NULL;
	pszPath = NULL;
	bStartEnabled = false;
	bReady = false;
	bIsEnabled = false;
	bOwnsSolvers = true;
}
EditorPostProcessingEffect::~EditorPostProcessingEffect()
{
	AllocCheck_Free();

	delete [] pszName;
	delete [] pszPath;

	if ( bOwnsSolvers )
		DestroySolverStack( hSolverArray );
	else
		hSolverArray.Purge();
}
EditorPostProcessingEffect::EditorPostProcessingEffect( const EditorPostProcessingEffect &o )
{
	AllocCheck_Alloc();

	pszName = NULL;
	pszPath = NULL;

	AutoCopyStringPtr( o.pszName, &pszName );
	AutoCopyStringPtr( o.pszPath, &pszPath );
	bStartEnabled = o.bStartEnabled;
	bIsEnabled = o.bIsEnabled;
	bReady = o.bReady;
	config = o.config;
	bOwnsSolvers = o.bOwnsSolvers;

	if ( o.hSolverArray.Count() )
		CopySolvers( o.hSolverArray, hSolverArray );
}
int EditorPostProcessingEffect::LoadPostProcessingChain( const char *Path, bool bStartEnabled )
{
	if ( !Path || !*Path )
		return CPostProcessingCache::PPE_INVALID_INPUT;

	delete [] pszName;
	delete [] pszPath;
	DestroySolverStack( hSolverArray );

	this->bStartEnabled = bStartEnabled;
	bIsEnabled = bStartEnabled;
	AutoCopyStringPtr( Path, &pszPath );

	char fileBase[MAX_PATH*4];
	Q_FileBase( Path, fileBase, sizeof( fileBase ) );

	if ( *fileBase )
	{
		int len = Q_strlen( fileBase ) + 1;
		pszName = new char[ len + 1 ];
		Q_strcpy( pszName, fileBase );
	}

	int retVal = CNodeView::CreatePPSolversFromFile( Path, hSolverArray, config );

	bReady = retVal == CPostProcessingCache::PPE_OKAY;

	return retVal;
}
void EditorPostProcessingEffect::ReplacePostProcessingChain( CUtlVector< CHLSL_SolverBase* > &hNewSolvers )
{
	DestroySolverStack( hSolverArray );
	CopySolvers( hNewSolvers, hSolverArray );
}
IMaterial *EditorPostProcessingEffect::FindMaterial( const char *pszNodeName )
{
	for ( int i = 0; i < hSolverArray.Count(); i++ )
	{
		CHLSL_Solver_PP_Mat *pMatSolver = dynamic_cast< CHLSL_Solver_PP_Mat* >( hSolverArray[i] );

		if ( !pMatSolver )
			continue;

		if ( !pMatSolver->GetNumTargetVars() )
			continue;

		IMaterial *pMat = pMatSolver->GetMaterial();

		if ( IsErrorMaterial( pMat ) )
			continue;

		if ( !pMatSolver->GetNodeName() || !*pMatSolver->GetNodeName() )
			continue;

		if ( Q_stricmp( pMatSolver->GetNodeName(), pszNodeName ) )
			continue;

		return pMat;
	}

	return NULL;
}





CPostProcessingCache::CPostProcessingCache() : CAutoGameSystem( "postprocessingcache" )
{
	m_bVrCallbacksLocked = false;
}

CPostProcessingCache::~CPostProcessingCache()
{
	hEffects.Purge();
	m_hVrCallbacks.PurgeAndDeleteElements();
}

bool CPostProcessingCache::Init()
{
	//LoadPrecacheFile();

	return true;
}

void CPostProcessingCache::Shutdown()
{
}

int CPostProcessingCache::GetNumPostProcessingEffects()
{
	return hEffects.Count();
}

EditorPostProcessingEffect *CPostProcessingCache::GetPostProcessingEffect( int num )
{
	return &hEffects[num];
}

int CPostProcessingCache::FindPostProcessingEffect( const char *name )
{
	for ( int i = 0; i < hEffects.Count(); i++ )
	{
		if ( !hEffects[i].pszName )
			continue;

		if ( !Q_stricmp( name, hEffects[i].pszName ) )
			return i;
	}
	return -1;
}

CPostProcessingCache::PPE_Error CPostProcessingCache::AddPostProcessingEffect( EditorPostProcessingEffect *effect )
{
	if ( !effect || !effect->pszName || !*effect->pszName )
		return PPE_INVALID_INPUT;

	for ( int i = 0; i < hEffects.Count(); i++ )
		if ( hEffects[i].pszName && !Q_stricmp( hEffects[i].pszName, effect->pszName ) )
			return PPE_INVALID_INPUT;

	hEffects.AddToTail( *effect );
	return PPE_OKAY;
}

CPostProcessingCache::PPE_Error CPostProcessingCache::LoadPostProcessingEffect( const char *path, bool bStartEnabled )
{
	EditorPostProcessingEffect effect;
	CPostProcessingCache::PPE_Error error = (CPostProcessingCache::PPE_Error)effect.LoadPostProcessingChain( path, bStartEnabled );

	if ( error != PPE_OKAY )
		return error;

	return AddPostProcessingEffect( &effect );
}

void CPostProcessingCache::DeletePostProcessingEffect( int index )
{
	if ( !hEffects.IsValidIndex( index ) )
		return;

	hEffects.Remove( index );
}

int PPESort( const EditorPostProcessingEffect *p1, const EditorPostProcessingEffect *p2 )
{
	if ( !(*p1).pszName || !(*p2).pszName )
		return 0;

	return Q_stricmp( (*p1).pszName, (*p2).pszName );
}
void CPostProcessingCache::SortPostProcessingEffects()
{
	hEffects.Sort( PPESort );
}
void CPostProcessingCache::MoveEffectAlongList( int index, bool bUp )
{
	if ( !hEffects.IsValidIndex( index ) )
		return;

	if ( index == 0 && bUp )
		return;
	if ( index == hEffects.Count() - 1 && !bUp )
		return;

	EditorPostProcessingEffect effect = hEffects[index];
	hEffects.Remove( index );

	if ( bUp )
		hEffects.InsertBefore( index - 1, effect );
	else
		hEffects.InsertAfter( index, effect );
}

void CPostProcessingCache::InitializeMaterials()
{
	CMatRenderContextPtr pRenderContext( materials );
	RunCodeContext rContext( false, false );
	rContext.pRenderContext = pRenderContext;

	for ( int i = 0; i < hEffects.Count(); i++ )
	{
		EditorPostProcessingEffect *effect = &hEffects[i];

		for ( int s = 0; s < effect->hSolverArray.Count(); s++ )
		{
			Assert( dynamic_cast< CHLSL_Solver_PP_Base* >( effect->hSolverArray[s] ) ||
				dynamic_cast< CHLSL_Solver_Dummy* >( effect->hSolverArray[s] ) );

			if ( dynamic_cast< CHLSL_Solver_PP_Mat* >( effect->hSolverArray[s] ) != NULL )
			{
				effect->hSolverArray[s]->Invoke_ExecuteCode( rContext );

				Assert( effect->hSolverArray[s]->GetNumTargetVars() > 0 &&
					!IsErrorMaterial( effect->hSolverArray[s]->GetTargetVar(0)->GetMaterial() ) );
			}
		}
	}
}

void CPostProcessingCache::RenderAllEffects( bool bScene )
{
	for ( int i = 0; i < hEffects.Count(); i++ )
	{
		if ( !hEffects[i].bReady )
			continue;

		if ( !hEffects[i].bIsEnabled )
			continue;

		RenderSinglePPE( &hEffects[i], false, bScene );
	}
}

void CPostProcessingCache::RenderSinglePPE( EditorPostProcessingEffect *effect, bool bPreviewMode, bool bSceneMode, bool bOwnsEffect )
{
	if ( !effect )
		return;

	CMatRenderContextPtr pRenderContext( materials );

	static ConVarRef r_queued_post_processing( "r_queued_post_processing", "1" );

#ifndef SHADER_EDITOR_DLL_2006
	if ( !bSceneMode && r_queued_post_processing.GetInt() )
	{
		ICallQueue *pCallQueue = pRenderContext->GetCallQueue();
		if ( pCallQueue )
		{
			AssertMsg( !bOwnsEffect, "Mem leak." );

			EditorPostProcessingEffect *pNewEffect = new EditorPostProcessingEffect( *effect );
			return pCallQueue->QueueCall( CPostProcessingCache::RenderSinglePPE, pNewEffect, bPreviewMode, bSceneMode, true );
		}
	}
#endif

	if ( !bSceneMode && effect->config.bDoAutoUpdateFBCopy )
	{
		GeneralFramebufferUpdate( pRenderContext );
	}


	RunCodeContext rContext( bPreviewMode, bSceneMode );
	rContext.pRenderContext = pRenderContext;

	for ( int i = 0; i < effect->hSolverArray.Count(); i++ )
	{
		Assert( dynamic_cast< CHLSL_Solver_PP_Base* >( effect->hSolverArray[i] ) ||
			dynamic_cast< CHLSL_Solver_Dummy* >( effect->hSolverArray[i] ) );

		effect->hSolverArray[i]->Invoke_ExecuteCode( rContext );
	}

	if ( bOwnsEffect )
	{
		delete effect;
	}
}

void CPostProcessingCache::RefreshAllPPEMaterials()
{
	for ( int i = 0; i < hEffects.Count(); i++ )
	{
		RefreshAllPPEMaterials( hEffects[i].hSolverArray );
	}

	if ( IsInEditMode() )
	{
		for ( int i = 0; i < pEditorRoot->GetNumFlowGraphs(); i++ )
		{
			CNodeView *pView = pEditorRoot->GetFlowGraph( i );

			if ( pView->GetFlowgraphType() != CNodeView::FLOWGRAPH_POSTPROC )
				continue;

			RefreshAllPPEMaterials( pView->AccessSolverStack_POSTPROC() );
		}
	}
}

void CPostProcessingCache::RefreshAllPPEMaterials( CUtlVector< CHLSL_SolverBase* > &hSolvers )
{
	for ( int i = 0; i < hSolvers.Count(); i++ )
	{
		CHLSL_Solver_PP_Mat *pMatSolver = dynamic_cast< CHLSL_Solver_PP_Mat* >( hSolvers[i] );
		if ( !pMatSolver )
			continue;

		if ( !pMatSolver->GetNumTargetVars() )
			continue;

		IMaterial *pMat = pMatSolver->GetTargetVar(0)->GetMaterial();

		if ( IsErrorMaterial( pMat ) )
		{
			Assert( 0 );
			return;
		}

		pMat->Refresh();
	}
}

const char *CPostProcessingCache::GetPPEffectPrecacheFile()
{
	static char tmp[MAX_PATH*4];
	Q_snprintf( tmp, sizeof(tmp), "%s/postprocessing_precache.txt", GetEditorRootDirectory() );

	return tmp;
}

void CPostProcessingCache::LoadPrecacheFile()
{
	KeyValues *pKV = new KeyValues( "pp_precache" );

	if ( pKV->LoadFromFile( g_pFullFileSystem, GetPPEffectPrecacheFile() ) )
	{
		for ( KeyValues *pSub = pKV->GetFirstTrueSubKey(); pSub; pSub = pSub->GetNextTrueSubKey() )
		{
			const char *pszEffectPath = pSub->GetString( "effect_path" );
			if ( !*pszEffectPath )
				continue;

			const bool bStartEnabled = !!pSub->GetInt( "effect_start_enabled" );

			EditorPostProcessingEffect effect;
			if ( effect.LoadPostProcessingChain( pszEffectPath, bStartEnabled ) != PPE_OKAY )
			{
				Warning( "Unable to precache post processing effect: %s!!!\n", pszEffectPath );
				continue;
			}

			if ( AddPostProcessingEffect( &effect ) != PPE_OKAY )
				Warning( "Unable to add post processing effect '%s' to precache list, duplicate names?\n", pszEffectPath );
		}
	}

	pKV->deleteThis();

	InitializeMaterials();
}

void CPostProcessingCache::SavePrecacheFile()
{
	KeyValues *pKV = new KeyValues( "pp_precache" );

	for ( int i = 0; i < hEffects.Count(); i++ )
	{
		EditorPostProcessingEffect *effect = &hEffects[i];

		if ( !effect->pszName || !effect->pszPath )
			continue;

		KeyValues *pKVEffect = new KeyValues( VarArgs("effect_%02i",i) );

		pKVEffect->SetString( "effect_path", effect->pszPath );
		pKVEffect->SetInt( "effect_start_enabled", effect->bStartEnabled ? 1 : 0 );

		pKV->AddSubKey( pKVEffect );
	}

	pKV->SaveToFile( g_pFullFileSystem, GetPPEffectPrecacheFile() );

	pKV->deleteThis();
}

void CPostProcessingCache::ClearInvalidEntries()
{
	for ( int i = 0; i < hEffects.Count(); i++ )
	{
		if ( !hEffects[i].pszName || !hEffects[i].pszPath ||
			!*hEffects[i].pszName || !*hEffects[i].pszPath ||
			!hEffects[i].bReady )
		{
			hEffects.Remove( i );
			i--;
		}
	}
}


void CPostProcessingCache::LockVrCallbacks()
{
	m_bVrCallbacksLocked = true;
}

bool CPostProcessingCache::AddVrCallback( EditorRenderViewCommand_Definition *pCallback )
{
	for ( int i = 0; i < m_hVrCallbacks.Count(); i++ )
	{
		if ( !Q_stricmp( pCallback->GetName(), m_hVrCallbacks[i]->GetName() ) )
		{
			delete pCallback;
			return false;
		}
	}

	m_hVrCallbacks.AddToTail( pCallback );
	return true;
}

int CPostProcessingCache::GetNumVrCallbacks()
{
	if ( !m_bVrCallbacksLocked )
		return 0;

	return m_hVrCallbacks.Count();
}

EditorRenderViewCommand_Definition *CPostProcessingCache::GetVrCallback( int index )
{
	if ( !m_bVrCallbacksLocked )
		return NULL;

	return m_hVrCallbacks[ index ];
}

int CPostProcessingCache::FindVrCallback( const char *szName )
{
	for ( int i = 0; i < m_hVrCallbacks.Count() && m_bVrCallbacksLocked; i++ )
		if ( !Q_stricmp( m_hVrCallbacks[i]->GetName(), szName ) )
			return i;
	return -1;
}