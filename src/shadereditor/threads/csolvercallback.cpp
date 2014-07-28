
#include "cbase.h"
#include "editorCommon.h"

//#include <Windows.h>

CUtlVector< CAutoSolverCallback* >CAutoSolverCallback::m_shClients;

CAutoSolverCallback::CAutoSolverCallback()
{
	m_bVSValid = false;
	m_bPSValid = false;

	m_iPreviewMode = PVMODE_FINAL_COMBINED;
	m_iRenderSystem = RENDERMODE_HARDWARE;

	pShaderData = NULL;

	static HAUTOSOLVER __incrementReq = 0;
	RequesterIDX = __incrementReq;
	__incrementReq++;
	m_shClients.AddToTail( this );
}
CAutoSolverCallback::~CAutoSolverCallback()
{
	m_shClients.FindAndRemove( this );

	delete pShaderData;
	//if ( m_hThread.IsValid() && IsInSoftwareMode() )
	//	gThreadCtrl->StopThread( m_hThread.Get(), true );

	m_hThread.Detach();
	DeleteSolverStacks();
}
HAUTOSOLVER CAutoSolverCallback::GetRequesterIDX()
{
	return RequesterIDX;
}
CAutoSolverCallback *CAutoSolverCallback::GetSolverClientByIdx( HAUTOSOLVER idx )
{
	for ( int i = 0; i < m_shClients.Count(); i++ )
	{
		if ( m_shClients[i]->GetRequesterIDX() == idx )
			return m_shClients[i];
	}
	return NULL;
}

#include "IVProcShader.h"

void CAutoSolverCallback::Update3DPrevMaterial( GenericShaderData *data, const int index )
{
	BasicShaderCfg_t *copy = data ? new BasicShaderCfg_t( *data->shader ) : NULL;

	Assert( !copy || copy->bPreviewMode );

	BasicShaderCfg_t *old = (BasicShaderCfg_t*)gProcShaderCTRL->SwapShaderSystem( copy, index );
	delete old;

	if ( index == 0 )
	{
		Assert( pEditorRoot );
		IMaterial *pTestMat = pEditorRoot->GetMainPreviewMat(); // materials->FindMaterial( "shadereditor/tex", TEXTURE_GROUP_OTHER );
		pTestMat->Refresh();
		pTestMat->RecomputeStateSnapshots();
	}
}

void CAutoSolverCallback::CopyShaders( int flags, GenericShaderData *data )
{
	Assert(data&&data->shader);

	//g_pFullFileSystem->fil
	char _vs_pos[MAX_PATH];
	char _vs[MAX_PATH];
	char _ps[MAX_PATH];
	char _vs_pos_tg[MAX_PATH];
	char _vs_tg[MAX_PATH];
	char _ps_tg[MAX_PATH];
	if ( flags & ACTIVEFLAG_VS_POS )
	{
		::ComposeShaderPath_Compiled( data, false, true, _vs_pos, MAX_PATH, true );
		::ComposeShaderPath_CompiledEngine( data, false, true, _vs_pos_tg, MAX_PATH, true );
		EngineCopy( _vs_pos, _vs_pos_tg );
	}
	if ( flags & ACTIVEFLAG_VS )
	{
		::ComposeShaderPath_Compiled( data, false, true, _vs, MAX_PATH );
		::ComposeShaderPath_CompiledEngine( data, false, true, _vs_tg, MAX_PATH );
		EngineCopy( _vs, _vs_tg );
	}
	if ( flags & ACTIVEFLAG_PS )
	{
		::ComposeShaderPath_Compiled( data, true, true, _ps, MAX_PATH );
		::ComposeShaderPath_CompiledEngine( data, true, true, _ps_tg, MAX_PATH );
		EngineCopy( _ps, _ps_tg );
	}
}
void CAutoSolverCallback::OnCompilationEnd( int flags, int exitcode, GenericShaderData *data )
{
	Assert(0);

	Msg("Compilation ended. Exitcode: %i\n", exitcode);
	Assert(data&&data->shader);
	CopyShaders( flags, data );
	if ( flags & ACTIVEFLAG_VS && flags & ACTIVEFLAG_PS )
		Update3DPrevMaterial( data );
	if ( flags & ACTIVEFLAG_VS )
		Update3DPrevMaterial( data, 1 );
	//delete data;
}
const bool CAutoSolverCallback::IsInSoftwareMode()
{
	return false;
}

void CAutoSolverCallback::DeleteSolverStacks()
{
	DestroySolverStack( m_hSolvers_VS );
	DestroySolverStack( m_hSolvers_PS );
	DestroySolverStack( m_hSolvers_UNDEF );
}

void CAutoSolverCallback::Startup()
{
	if ( !m_hThread.IsValid() )
	{
		//if ( IsInSoftwareMode() )
		//	m_hThread.Attach( gThreadCtrl->CreateSolverThread() );
		//else
			m_hThread.Attach( gThreadCtrl->GetCompileThread() );
	}
}
void CAutoSolverCallback::Shutdown()
{
	if ( !m_hThread.IsValid() )
		return;

	//if ( IsInSoftwareMode() )
	//	gThreadCtrl->StopThread( m_hThread.Get(), false );
	m_hThread.Detach();
}
bool CAutoSolverCallback::IsRunning()
{
	return m_hThread.IsValid() && m_hThread.Get()->IsAlive();
}
void CAutoSolverCallback::OnSolverUpdated( CNodeView *pCaller, GenericShaderData *pData, bool VSReady, bool PSReady )
{
	m_bVSValid = VSReady;
	m_bPSValid = PSReady;

	DeleteSolverStacks();

	CopySolvers( pCaller->AccessSolverStack_VS(), m_hSolvers_VS );
	CopySolvers( pCaller->AccessSolverStack_PS(), m_hSolvers_PS );
	CopySolvers( pCaller->AccessSolverStack_UNDEFINED(), m_hSolvers_UNDEF );

	if ( GetPreviewMode() == PVMODE_FINAL_COMBINED )
	{
		DestroySolverStack( m_hSolvers_UNDEF );
	}

	delete pShaderData;
	pShaderData = new GenericShaderData( *pData );

	if ( !IsRunning() )
		Startup();

	UploadCompilationRequest();

	if ( !m_hThread->IsAlive() )
		m_hThread->Start();
}
void CAutoSolverCallback::UploadCompilationRequest()
{
	Assert( pShaderData );

	__threadcmds_CompileCommand *msg = new __threadcmds_CompileCommand( GetRequesterIDX() );
	const bool bShaderInfo = !!pShaderData;
	const bool bAllowInvalidHierachies = false;

	if ( (IsVSReady()||bAllowInvalidHierachies) && m_hSolvers_VS.Count() && bShaderInfo )
	{
		msg->__vs = new CUtlVector< CHLSL_SolverBase* >;
		CopySolvers( m_hSolvers_VS, *((CUtlVector< CHLSL_SolverBase* >*)msg->__vs) );
	}

	if ( (IsPSReady()||bAllowInvalidHierachies) && m_hSolvers_PS.Count() && bShaderInfo )
	{
		msg->__ps = new CUtlVector< CHLSL_SolverBase* >;
		CopySolvers( m_hSolvers_PS, *((CUtlVector< CHLSL_SolverBase* >*)msg->__ps) );
	}

	if ( m_hSolvers_UNDEF.Count() && bShaderInfo )
	{
		msg->__undef = new CUtlVector< CHLSL_SolverBase* >;
		CopySolvers( m_hSolvers_UNDEF, *((CUtlVector< CHLSL_SolverBase* >*)msg->__undef) );
	}

	if ( bShaderInfo )
		msg->__data = new GenericShaderData( *pShaderData );

#if SHOW_SEDIT_ERRORS
	bool bHasLookupSolver = false;
	bool bHasTextureIdentifier = msg->__data && !!((GenericShaderData*)msg->__data)->shader->pPS_Identifiers->hList_Textures.Count();
	if ( msg->__ps )
	{
		CUtlVector< CHLSL_SolverBase* > &solvers = *((CUtlVector< CHLSL_SolverBase* >*)msg->__ps);
		for ( int i = 0; i < solvers.Count(); i++ )
			if ( dynamic_cast< CHLSL_Solver_TextureSample* >( solvers[i] ) )
				bHasLookupSolver = true;
	}
	Assert( !bHasLookupSolver || bHasLookupSolver == bHasTextureIdentifier );
#endif

	CCompileThread *cThread = (CCompileThread*)m_hThread.Get();
	cThread->m_Queue_CompileCommand.QueueMessage( msg );
}

void CAutoSolverCallback::OnSolverErrored()
{
}
const bool CAutoSolverCallback::IsVSReady()
{
	return m_bVSValid;
}
const bool CAutoSolverCallback::IsPSReady()
{
	return m_bPSValid;
}
void CAutoSolverCallback::SetRenderSystem( const int t )
{
}
const int CAutoSolverCallback::GetRenderSystem()
{
	return m_iRenderSystem;
}
void CAutoSolverCallback::SetPreviewMode( const int t )
{
	m_iPreviewMode = t;
}
const int CAutoSolverCallback::GetPreviewMode()
{
	return m_iPreviewMode;
}