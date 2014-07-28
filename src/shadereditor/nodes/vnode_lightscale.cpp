
#include "cbase.h"
#include "editorCommon.h"


CNodeLightscale::CNodeLightscale( CNodeView *p ) : BaseClass( "Lightscale", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_FLOAT1, "" );
	m_iLightScale = LSCALE_LINEAR;
	UpdateNode();
}

CNodeLightscale::~CNodeLightscale()
{
}

void CNodeLightscale::UpdateNode()
{
	GetJack_Out( 0 )->SetName( GetLightscaleGameString( m_iLightScale ) );
}


KeyValues *CNodeLightscale::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_lscale_type", m_iLightScale );
	return pKV;
}
void CNodeLightscale::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_iLightScale = pKV->GetInt( "i_lscale_type", m_iLightScale );
	UpdateNode();
}

bool CNodeLightscale::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	SetAllocating( false );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	tg->MakeConstantOnly();

	CHLSL_Solver_Lightscale *solver = new CHLSL_Solver_Lightscale( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	solver->SetState( m_iLightScale );
	AddSolver( solver );
	return true;
}