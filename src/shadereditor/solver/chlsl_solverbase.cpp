
#include "cbase.h"
#include "editorcommon.h"

SolverInfo_t::SolverInfo_t()
{
	iNodeIndex = 0;
	m_bValid = true;
}
SolverInfo_t::~SolverInfo_t()
{
	_IntValue_0 = _IntValue_1 = 0;
}
SolverInfo_t::SolverInfo_t( const SolverInfo_t &o )
{
	iNodeIndex = o.iNodeIndex;
	_IntValue_0 = o._IntValue_0;
	_IntValue_1 = o._IntValue_1;
	m_bValid = o.m_bValid;
}

CHLSL_SolverBase::CHLSL_SolverBase( HNODE nodeidx )
{
	m_iResourceType = RESOURCETYPE_VARIABLE;

	//pInfo = new SolverInfo_t();
	//pInfo->iNodeIndex = nodeidx;
	pInfo.iNodeIndex = nodeidx;
	m_iMapIndex = 0;

	AllocCheck_Alloc();
}

CHLSL_SolverBase::~CHLSL_SolverBase()
{
	m_hVar_SourceVars.Purge();
	m_hVar_TargetVars.Purge();

	//delete pInfo;

	AllocCheck_Free();
}

CHLSL_SolverBase::CHLSL_SolverBase( const CHLSL_SolverBase& o )
{
	m_iResourceType = o.m_iResourceType;
	m_iMapIndex = 0;

	m_hVar_SourceVars.Purge();
	m_hVar_TargetVars.Purge();

	pInfo = o.pInfo;
	//pInfo = NULL;

	//if ( o.pInfo )
	//	pInfo = new SolverInfo_t( *o.pInfo );

	AllocCheck_Alloc();
}

void CHLSL_SolverBase::ClearInfo()
{
	//delete pInfo;
	//pInfo = NULL;
	pInfo.m_bValid = false;
}

SolverInfo_t &CHLSL_SolverBase::GetData()
{
	Assert( pInfo.m_bValid );
	return pInfo;
	//Assert( pInfo );
	//return *pInfo;
}

const ResourceType_t &CHLSL_SolverBase::GetResourceType()
{
	return m_iResourceType;
}
void CHLSL_SolverBase::SetResourceType( const int &type )
{
	m_iResourceType = ((ResourceType_t)type);
}

void CHLSL_SolverBase::AddSourceVar( CHLSL_Var *var )
{
	//if ( m_hVar_SourceVars.IsValidIndex( m_hVar_SourceVars.Find( var ) ) )
	//	return;

	m_hVar_SourceVars.AddToTail( var );
}
void CHLSL_SolverBase::AddTargetVar( CHLSL_Var *var )
{
	//if ( m_hVar_TargetVars.IsValidIndex( m_hVar_TargetVars.Find( var ) ) )
	//	return;

	m_hVar_TargetVars.AddToTail( var );
}
int CHLSL_SolverBase::GetNumSourceVars()
{
	return m_hVar_SourceVars.Count();
}
int CHLSL_SolverBase::GetNumTargetVars()
{
	return m_hVar_TargetVars.Count();
}
CHLSL_Var *CHLSL_SolverBase::GetSourceVar( int idx )
{
	Assert( m_hVar_SourceVars.IsValidIndex( idx ) );
	return m_hVar_SourceVars[ idx ];
}
CHLSL_Var *CHLSL_SolverBase::GetTargetVar( int idx )
{
	Assert( m_hVar_TargetVars.IsValidIndex( idx ) );
	return m_hVar_TargetVars[ idx ];
}
void CHLSL_SolverBase::ReconnectVariables( CHLSL_SolverBase *shadow,
											CUtlVector< CHLSL_Var* > &m_hListOld,
											CUtlVector< CHLSL_Var* > &m_hListNew )
{
	for ( int i = 0; i < shadow->m_hVar_SourceVars.Count(); i++ )
	{
		int idx = m_hListOld.Find( shadow->m_hVar_SourceVars[i] );
		if ( !m_hListOld.IsValidIndex( idx ) )
			Warning("solver copy: FATAL ERROR\n");

		m_hVar_SourceVars.AddToTail( m_hListNew[idx] );
	}

	for ( int i = 0; i < shadow->m_hVar_TargetVars.Count(); i++ )
	{
		int idx = m_hListOld.Find( shadow->m_hVar_TargetVars[i] );
		if ( !m_hListOld.IsValidIndex( idx ) )
			Warning("solver copy: FATAL ERROR\n");

		m_hVar_TargetVars.AddToTail( m_hListNew[idx] );
	}

	Assert( shadow->GetNumSourceVars() == GetNumSourceVars() );
	Assert( shadow->GetNumTargetVars() == GetNumTargetVars() );
}

void CHLSL_SolverBase::Invoke_VarInit( const WriteContext_FXC &context )
{
	OnVarInit( context );
}
void CHLSL_SolverBase::OnVarInit( const WriteContext_FXC &context )
{
}
void CHLSL_SolverBase::Invoke_VarInit_PostStep()
{
	OnVarInit_PostStep();
}
void CHLSL_SolverBase::OnVarInit_PostStep()
{
}

void CHLSL_SolverBase::Invoke_WriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	OnWriteFXC( bIsPixelShader, context );
}
void CHLSL_SolverBase::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
}

void CHLSL_SolverBase::Invoke_Render( Preview2DContext &c )
{
	Render( c );
	m_iMapIndex = ::GetCurrentPuzzleIndex();
}
void CHLSL_SolverBase::Render( Preview2DContext &c )
{
	if ( GetNumTargetVars() < 1 )
		return;

	float info_0[4] = { 1, 1, 1, 1 };
	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( info_0, 4 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_SolverBase::UpdateTargetVarToReflectMapIndex( int var )
{
	if ( var > (GetNumTargetVars()-1) )
	{
		//Assert( 0 );
		return;
	}

	CHLSL_Var *target = GetTargetVar( var );
	target->SetMapIndex( GetCurrentPuzzleIndex() );
	//Msg( "setting idx: %i\n", GetCurrentPuzzleIndex() );
}
void CHLSL_SolverBase::SetUVParamBySourceVar( int operatoridx, int uvparam, int sourcevar )
{
	Assert( sourcevar < GetNumSourceVars() );

	CHLSL_Var *src1 = GetSourceVar( sourcevar );
	IMaterialVar *pMatVar = pEditorRoot->GetUVTargetParam( operatoridx, uvparam );
	Vector2D a,b;
	GetUVsForPuzzle( src1->GetMapIndex(), a, b );
	//Msg("index %i, uv coords: %f %f\n", src1->GetMapIndex(), a.x, a.y);
	pMatVar->SetVecValue( a.x, a.y, b.x, b.y );
}

void CHLSL_SolverBase::Invoke_RenderInit()
{
	OnRenderInit();
}
void CHLSL_SolverBase::OnRenderInit()
{
}

void CHLSL_SolverBase::Invoke_IdentifierAllocate( IdentifierLists_t &List )
{
	OnIdentifierAlloc( List );
}
void CHLSL_SolverBase::OnIdentifierAlloc( IdentifierLists_t &List )
{
}

void CHLSL_SolverBase::Invoke_ExecuteCode( const RunCodeContext &context )
{
	OnExecuteCode( context );
}
void CHLSL_SolverBase::OnExecuteCode( const RunCodeContext &context )
{
}

CHLSL_SolverBase *CHLSL_SolverBase::Copy()
{
#if SHOW_SEDIT_ERRORS
	AssertMsg( 0, "Copy called on baseSolver!!\n" );
#endif
	return NULL;
}