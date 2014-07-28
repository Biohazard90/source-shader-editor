
#include "cbase.h"
#include "editorCommon.h"


CNodeEnvC::CNodeEnvC( CNodeView *p, float defaultSmartVal ) : BaseClass( "UNKNOWN ENVC", p )
{
	m_iNodeType = HLSLNODE_CONSTANT_TIME;
	m_iEnvCIndex = 0;

	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	m_flSmartVal = m_flDefaultSmartVal = defaultSmartVal;
}
CNodeEnvC::~CNodeEnvC()
{
}
int CNodeEnvC::GetAllowedHierachiesAsFlags()
{
	switch ( m_iEnvCIndex )
	{
	case HLSLENV_FOG_PARAMS:
			return HLSLHIERACHY_PS;
		break;
	}

	return BaseClass::GetAllowedHierachiesAsFlags();
}

KeyValues *CNodeEnvC::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetFloat( "flSmartVal0", m_flSmartVal );

	return pKV;
}
void CNodeEnvC::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_flSmartVal = pKV->GetFloat( "flSmartVal0" );
}

void CNodeEnvC::SetEnvCType( int type )
{
	m_iEnvCIndex = type;
	m_iNodeType = EConstant_GetData(type)->nodetype;

	SetName( EConstant_GetData(type)->nodename );

	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, EConstant_GetData(type)->varflag, EConstant_GetData(type)->jackname );
}
bool CNodeEnvC::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );

	const int res = pJ_Out->GetResourceType();

	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	tg->MakeConstantOnly();

	CHLSL_Solver_EnvC *solver = new CHLSL_Solver_EnvC( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	solver->SetEnvironmentConstantIndex( m_iEnvCIndex, m_flSmartVal );
	AddSolver( solver );

	return true;
}
