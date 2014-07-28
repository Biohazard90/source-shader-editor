
#include "cbase.h"
#include "editorCommon.h"

CNodePP_RT::CNodePP_RT( CNodeView *p ) : BaseClass( "Rendertarget", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_RT, "RT" );

	m_szRTName[0] = '\0';

	OnUpdateHierachy(NULL,NULL);
}

CNodePP_RT::~CNodePP_RT()
{
}

KeyValues *CNodePP_RT::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetString( "szRTName", m_szRTName );

	return pKV;
}

void CNodePP_RT::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	Q_snprintf( m_szRTName, sizeof(m_szRTName), "%s", pKV->GetString( "szRTName" ) );

	OnUpdateHierachy(NULL,NULL);
}

int CNodePP_RT::UpdateInputsValid()
{
	if ( Q_strlen( m_szRTName ) < 1 )
		return ERRORLEVEL_UNDEFINED;
	return ERRORLEVEL_NONE;
}

bool CNodePP_RT::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( Q_strlen( m_szRTName ) < 1 )
		Assert( 0 );

	CHLSL_Var *pVar = GetJack_Out(0)->AllocateVarFromSmartType();

	CHLSL_Solver_PP_RT *solver = new CHLSL_Solver_PP_RT( GetUniqueIndex() );
	solver->AddTargetVar( pVar );
	solver->Init( m_szRTName );
	AddSolver( solver );
	return true;
}