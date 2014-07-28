
#include "cbase.h"
#include "editorCommon.h"

static CMaterialSysDeletionHelper gMatSysDelHint;
CMaterialSysDeletionHelper *GetMatSysShutdownHelper(){ return &gMatSysDelHint;};

RTDef::RTDef()
{
	szRTName[0] = '\0';
	ref = NULL;
	Format = IMAGE_FORMAT_RGBA8888;
	DepthMode = MATERIAL_RT_DEPTH_NONE;
	SizeMode = RT_SIZE_NO_CHANGE;
	x = y = RT_DEFAULT_SIZE;

	bDirty = false;
	bFlaggedForDeletion = false;
}

RTDef::RTDef( const char *pszName,
	ImageFormat format, RenderTargetSizeMode_t sizemode, MaterialRenderTargetDepth_t depthmode,
	int x, int y, unsigned int rtflags )
{
	Assert( pszName && *pszName );

	Q_snprintf( szRTName, sizeof(szRTName), "%s", pszName );
	ref = NULL;
	Format = format;
	DepthMode = depthmode;
	SizeMode = sizemode;
	iRTFlags = rtflags;
	this->x = x;
	this->y = y;

	bDirty = true;
	bFlaggedForDeletion = false;
	//ref.InitRenderTarget( 256, 256, sizemode, format, depthmode, false, pszName );
}

RTDef::~RTDef()
{
	//if ( !GetMatSysShutdownHelper()->IsShuttingDown() && ref.IsValid() )
	//	ref.Shutdown();
	//ref.Shutdown( true );
}

const char *RTDef::GetName() const
{
	return szRTName;
}
ImageFormat RTDef::GetImgFormat()
{
	return Format;
}
RenderTargetSizeMode_t RTDef::GetSizeMode()
{
	return SizeMode;
}
MaterialRenderTargetDepth_t RTDef::GetDepthMode()
{
	return DepthMode;
}
unsigned int RTDef::GetFlags()
{
	return iRTFlags;
}
int RTDef::GetSizeX()
{
	return x;
}
int RTDef::GetSizeY()
{
	return y;
}

bool RTDef::IsValid()
{
	return ref != NULL; //ref.IsValid();
}

void RTDef::Allocate()
{
	Assert( *GetName() );

	RenderTargetSizeMode_t local_sizemode = SizeMode;

	if ( local_sizemode == RT_SIZE_HALF_FRAME_BUFFER )
		local_sizemode = RT_SIZE_NO_CHANGE;

	ITexture *pTex = materials->CreateNamedRenderTargetTextureEx( szRTName,
		x, y, SizeMode, Format, DepthMode, iRTFlags, 0 );
	//ref.Init( pTex );
	ref = pTex;

	bDirty = false;
}

void RTDef::Deallocate()
{
	if ( ref != NULL ) //ref.IsValid() )
	{
		//ITexture *pTex = ref;
		//ref.Shutdown( false );

		//ForceDeleteTexture( &pTex );

		//ForceDeleteTexture( &ref );
		//ref.Shutdown( true );

		ref = NULL;
	}
}

void RTDef::Modify( RTDef &src )
{
	Modify( src.GetName(), &src.Format, &src.SizeMode, &src.DepthMode, &src.x, &src.y, &src.iRTFlags );
}

void RTDef::Modify( const char *pszName,
		ImageFormat *format, RenderTargetSizeMode_t *sizemode, MaterialRenderTargetDepth_t *depthmode,
		int *x, int *y, unsigned int *rtflags )
{
	bool bChanges = false;
	if ( pszName != NULL && Q_strlen( pszName ) )
	{
		bChanges = bChanges || Q_strcmp( szRTName, pszName );
		Q_snprintf( szRTName, sizeof(szRTName), "%s", pszName );
	}

	if ( format != NULL )
	{
		bChanges = bChanges || Format != *format;
		Format = *format;
	}

	if ( depthmode != NULL )
	{
		bChanges = bChanges || DepthMode != *depthmode;
		DepthMode = *depthmode;
	}

	if ( sizemode != NULL )
	{
		bChanges = bChanges || SizeMode != *sizemode;
		SizeMode = *sizemode;
	}

	if ( rtflags != NULL )
	{
		bChanges = bChanges || iRTFlags != *rtflags;
		iRTFlags = *rtflags;
	}

	if ( x != NULL )
	{
		bChanges = bChanges || this->x != *x;
		this->x = *x;
	}

	if ( y != NULL )
	{
		bChanges = bChanges || this->y != *y;
		this->y = *y;
	}

	if ( bChanges )
		MakeDirty();
}

bool RTDef::IsDirty()
{
	return bDirty;
}

void RTDef::MakeDirty()
{
	bDirty = true;
}

bool RTDef::IsFlaggedForDeletion()
{
	return bFlaggedForDeletion;
}

void RTDef::FlagForDeletion()
{
	bFlaggedForDeletion = true;
}

void RTDef::Revalidate()
{
	bFlaggedForDeletion = false;
}

static RenderTargetManager gs_RenderTargetManager;
RenderTargetManager *GetRTManager(){ return &gs_RenderTargetManager; };

//CUtlVector< RTDef > RenderTargetManager::m_hRTs;

void UpdateCustomAutoSizing()
{
	int bbx, bby;
	materials->GetBackBufferDimensions( bbx, bby );

	int halfx = bbx / 2;
	int halfy = bby / 2;

	for ( int i = 0; i < GetRTManager()->GetNumRTs(); i++ )
	{
		RTDef *rt = GetRTManager()->GetRT(i);

		if ( rt->GetSizeMode() == RT_SIZE_HALF_FRAME_BUFFER )
		{
			rt->Modify( NULL, NULL, NULL, NULL, &halfx, &halfy );
		}
	}
}

#ifdef SHADER_EDITOR_DLL_2006
void EditorRTs_Release()
{
	GetRTManager()->MakeRTCacheDirty();
}
void EditorRTs_Restore( int nChangeFlags )
{
	GetRTManager()->MakeRTCacheDirty();
}
#else
void EditorRTs_ModeChangeCallback()
{
	GetRTManager()->MakeRTCacheDirty();
}
#endif

RenderTargetManager::RenderTargetManager() : CAutoGameSystemPerFrame( "rtmanagersys" )
{
	m_bCacheDirty = false;
}

RenderTargetManager::~RenderTargetManager()
{
	//Assert( !m_hRTs.Count() );
}

const char *RenderTargetManager::GetRTSetupFilePath()
{
	static char rtpath[MAX_PATH*4];
	Q_snprintf( rtpath, sizeof(rtpath), "%s/rtsetup.txt", GetEditorRootDirectory() );
	return rtpath;
}

bool RenderTargetManager::Init()
{
	KeyValues *pRTSetup = new KeyValues( "rt_setup" );

	if ( pRTSetup->LoadFromFile( g_pFullFileSystem, GetRTSetupFilePath() ) )
	{
		for ( KeyValues *pSub = pRTSetup->GetFirstTrueSubKey(); pSub; pSub = pSub->GetNextTrueSubKey() )
		{
			const char *pszName = pSub->GetString( "rt_name" );

			if ( !pszName || !*pszName )
				continue;

			ImageFormat format = (ImageFormat)pSub->GetInt( "rt_imgformat", IMAGE_FORMAT_RGBA8888 );
			RenderTargetSizeMode_t sizemode = (RenderTargetSizeMode_t)pSub->GetInt( "rt_sizemode", RT_SIZE_FULL_FRAME_BUFFER );
			MaterialRenderTargetDepth_t depthmode = (MaterialRenderTargetDepth_t)pSub->GetInt( "rt_depthmode", MATERIAL_RT_DEPTH_NONE );
			unsigned int flags = (unsigned int)pSub->GetInt( "rt_flags", 0 );
			int x = pSub->GetInt( "rt_size_x", 0 );
			int y = pSub->GetInt( "rt_size_y", 0 );

			RTDef rt( pszName, format, sizemode, depthmode, x, y, flags );
			m_hRTs.AddToTail( rt );
		}
	}

	pRTSetup->deleteThis();

#ifdef SHADER_EDITOR_DLL_2006
	materials->AddReleaseFunc( &EditorRTs_Release );
	materials->AddRestoreFunc( &EditorRTs_Restore );
#else
	materials->AddModeChangeCallBack( &EditorRTs_ModeChangeCallback );
#endif
	UpdateRTsToMatSys( true );

	return true;
}

void RenderTargetManager::Shutdown()
{
	for ( int i = 0; i < m_hRTs.Count(); i++ )
		m_hRTs[i].FlagForDeletion();

	UpdateRTsToMatSys();
#ifdef SHADER_EDITOR_DLL_2006
	materials->RemoveReleaseFunc( &EditorRTs_Release );
	materials->RemoveRestoreFunc( &EditorRTs_Restore );
#else
	materials->RemoveModeChangeCallBack( &EditorRTs_ModeChangeCallback );
#endif
}

void RenderTargetManager::PreRender()
{
	if ( m_bCacheDirty )
	{
		m_bCacheDirty = false;

		UpdateCustomAutoSizing();
		UpdateRTsToMatSys();
	}
}

void RenderTargetManager::MakeRTCacheDirty()
{
	m_bCacheDirty = true;
}

void RenderTargetManager::SaveRTsToFile()
{
	KeyValues *pRTSetup = new KeyValues( "rt_setup" );

	for ( int i = 0; i < m_hRTs.Count(); i++ )
	{
		RTDef *rt = &m_hRTs[i];

		if ( !Q_strlen( rt->GetName() ) )
			continue;

		if ( rt->IsFlaggedForDeletion() )
			continue;

		KeyValues *pSub = new KeyValues( VarArgs("rt_%02i",i) );

		pSub->SetString( "rt_name", rt->GetName() );
		pSub->SetInt( "rt_imgformat", rt->GetImgFormat() );
		pSub->SetInt( "rt_sizemode", rt->GetSizeMode() );
		pSub->SetInt( "rt_depthmode", rt->GetDepthMode() );
		pSub->SetInt( "rt_flags", rt->GetFlags() );
		pSub->SetInt( "rt_size_x", rt->GetSizeX() );
		pSub->SetInt( "rt_size_y", rt->GetSizeY() );

		pRTSetup->AddSubKey( pSub );
	}

	pRTSetup->SaveToFile( g_pFullFileSystem, GetRTSetupFilePath() );
	pRTSetup->deleteThis();
}

void RenderTargetManager::UpdateRTsToMatSys( bool bOnStartup )
{
	bool bHasDirtyRTs = false;

	for ( int i = 0; i < m_hRTs.Count() && !bHasDirtyRTs; i++ )
	{
		RTDef *rt = &m_hRTs[i];

		bHasDirtyRTs = /*rt->IsFlaggedForDeletion() ||*/ rt->IsDirty();
	}

	if ( !bHasDirtyRTs )
		return;

	if ( GetMatSysShutdownHelper()->IsShuttingDown() )
	{
		m_hRTs.Purge();
		return;
	}

	UpdateCustomAutoSizing();

#if 0
	materials->BeginRenderTargetAllocation();
	for ( int i = 0; i < m_hRTs.Count(); i++ )
	{
		RTDef *rt = &m_hRTs[i];

		if ( rt->IsFlaggedForDeletion() )
		{
			rt->Deallocate();
			m_hRTs.Remove( i );
			i--;
			continue;
		}

		//Assert( rt->IsValid() );
	}
	materials->EndRenderTargetAllocation();
#endif

#ifdef SHADER_EDITOR_DLL_SWARM
	if ( !bOnStartup )
	{
		// srsly? SRSLY?
		materials->ReEnableRenderTargetAllocation_IRealizeIfICallThisAllTexturesWillBeUnloadedAndLoadTimeWillSufferHorribly();
	}
#endif

	materials->BeginRenderTargetAllocation();
	for ( int i = 0; i < m_hRTs.Count(); i++ )
	{
		RTDef *rt = &m_hRTs[i];

		if ( rt->IsDirty() )
		{
			rt->Deallocate();
			rt->Allocate();
		}

		Assert( rt->IsValid() );
	}
	materials->EndRenderTargetAllocation();

#ifdef SHADER_EDITOR_DLL_SWARM
	engine->ClientCmd_Unrestricted( "mat_reloadallmaterials" );
#endif
}

int RenderTargetManager::GetNumRTs()
{
	return m_hRTs.Count();
}

RTDef *RenderTargetManager::GetRT( int id )
{
	return &m_hRTs[id];
}

void RenderTargetManager::RemoveRT( int index )
{
	m_hRTs.Remove( index );
}

void RenderTargetManager::AddRT( RTDef rt )
{
	m_hRTs.AddToTail( rt );
}

int RTSort( const RTDef *p1, const RTDef *p2 )
{
	return Q_stricmp( (*p1).GetName(), p2->GetName() );
}
void RenderTargetManager::SortRTsByName()
{
	m_hRTs.Sort( RTSort );
}

RTDef *RenderTargetManager::FindRTByName( const char *name, bool bIgnoreFlaggedForDel )
{
	for ( int i = 0; i < m_hRTs.Count(); i++ )
		if ( !Q_stricmp( name, m_hRTs[i].GetName() ) &&
			( !bIgnoreFlaggedForDel || !m_hRTs[i].IsFlaggedForDeletion() ) )
			return &m_hRTs[i];

	return NULL;
}

int RenderTargetManager::FindRTIndexByName( const char *name, bool bIgnoreFlaggedForDel )
{
	for ( int i = 0; i < m_hRTs.Count(); i++ )
		if ( !Q_stricmp( name, m_hRTs[i].GetName() ) &&
			( !bIgnoreFlaggedForDel || !m_hRTs[i].IsFlaggedForDeletion() ) )
			return i;

	return -1;
}