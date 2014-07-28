
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_VParam_Mutable::CHLSL_Solver_VParam_Mutable( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx )
{
	m_ConstantData.SetDefaultChannels();
}

void CHLSL_Solver_VParam_Mutable::SetComponentInfo( const TokenChannels_t &info )
{
	if ( info.IsActive() )
		m_ConstantData = info;
}

void CHLSL_Solver_VParam_Mutable::SetState( int pIdx, int comps )
{
	m_iParamIndex = pIdx;
	m_iNumComponents = comps;
}

void CHLSL_Solver_VParam_Mutable::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];

	Q_snprintf( tmp, MAXTARGC, "g_cData_Mutable_%02i", m_iParamIndex + 1 );
	target->SetName( tmp, true );
}

void CHLSL_Solver_VParam_Mutable::OnIdentifierAlloc( IdentifierLists_t &List )
{
	Assert( m_iParamIndex >= 0 && m_iParamIndex <= (AMT_VMT_MUTABLE-1) );

	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_SMART_VMT_MUTABLE;
	ec->iHLSLRegister = -1;
	ec->iSmartNumComps = m_iNumComponents;

	char tmp[MAX_PATH];
	Q_snprintf( tmp, MAX_PATH, "Mutable_%02i", m_iParamIndex + 1 );
	ec->szSmartHelper = new char[Q_strlen(tmp)+1];
	Q_strcpy( ec->szSmartHelper, tmp );
	for ( int i = 0; i < m_ConstantData.iActiveChannels && i < 4; i++ )
	{
		Assert( m_ConstantData.bChannelAsValue[ i ] );
		ec->flSmartDefaultValues[ i ] = m_ConstantData.flValue[ i ];
	}

	List.hList_EConstants.AddToTail( ec );
}

void CHLSL_Solver_VParam_Mutable::Render( Preview2DContext &c )
{
	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( 0, 0, 0, 0 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}





CHLSL_Solver_VParam_Static::CHLSL_Solver_VParam_Static( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx )
{
	m_szName = NULL;
	m_ConstantData.SetDefaultChannels();
}
CHLSL_Solver_VParam_Static::~CHLSL_Solver_VParam_Static()
{
	delete [] m_szName;
}
CHLSL_Solver_VParam_Static::CHLSL_Solver_VParam_Static( const CHLSL_Solver_VParam_Static &o ) : CHLSL_SolverBase( o )
{
	m_iNumComponents = o.m_iNumComponents;
	m_ConstantData = o.m_ConstantData;

	Assert( o.m_szName && Q_strlen( o.m_szName ) > 0 );
	m_szName = new char[ Q_strlen( o.m_szName ) + 1 ];
	Q_strcpy( m_szName, o.m_szName );
}
void CHLSL_Solver_VParam_Static::SetComponentInfo( const TokenChannels_t &info )
{
	if ( info.IsActive() )
		m_ConstantData = info;
}
void CHLSL_Solver_VParam_Static::SetState( char *name, int comps )
{
	Assert( name && Q_strlen( name ) );
	delete [] m_szName;
	m_szName = new char[ Q_strlen( name ) + 1 ];
	Q_strcpy( m_szName, name );
	m_iNumComponents = comps;
}
void CHLSL_Solver_VParam_Static::OnVarInit( const WriteContext_FXC &context )
{
	Assert( m_szName );
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];

	Q_snprintf( tmp, MAXTARGC, "g_cData_%s", m_szName );
	target->SetName( tmp, true );
}
void CHLSL_Solver_VParam_Static::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_SMART_VMT_STATIC;
	ec->iHLSLRegister = -1;
	ec->iSmartNumComps = m_iNumComponents;

	char tmp[MAX_PATH];
	Q_snprintf( tmp, MAX_PATH, "%s", m_szName );
	ec->szSmartHelper = new char[Q_strlen(tmp)+1];
	for ( int i = 0; i < m_ConstantData.iActiveChannels && i < 4; i++ )
	{
		Assert( m_ConstantData.bChannelAsValue[ i ] );
		ec->flSmartDefaultValues[ i ] = m_ConstantData.flValue[ i ];
	}
	Q_strcpy( ec->szSmartHelper, tmp );

	List.hList_EConstants.AddToTail( ec );
}
void CHLSL_Solver_VParam_Static::Render( Preview2DContext &c )
{
	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( 0, 0, 0, 0 );
	if ( m_ConstantData.iActiveChannels )
		pMatVar0->SetVecValue( m_ConstantData.flValue, m_ConstantData.iActiveChannels );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}