
#include "cbase.h"
#include "editorCommon.h"


CNodeVmtParam_Mutable::CNodeVmtParam_Mutable( CNodeView *p ) : BaseClass( "VParam mutable", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	GenerateJacks_Output( 1 );

	m_iParamIndex = 0;
	m_iNumComponents = 3;
	m_iChannelInfo.SetDefaultChannels();
	m_iChannelInfo.bAllowChars = false;
	m_szConstantString[0] = '\0';

	UpdateNode();
}
void CNodeVmtParam_Mutable::UpdateNode()
{
	LockJackOutput_Flags( 0, ::GetVarTypeFlag( m_iNumComponents ), VarArgs( "Idx %02i", m_iParamIndex + 1 ) );

	OnUpdateHierachy( NULL, NULL );
}
KeyValues *CNodeVmtParam_Mutable::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetInt( "i_vp_mutable_param", m_iParamIndex );
	pKV->SetInt( "i_vp_mutable_nc", m_iNumComponents );
	pKV->SetString( "szConstantString", m_szConstantString );

	return pKV;
}
void CNodeVmtParam_Mutable::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_iParamIndex = pKV->GetInt( "i_vp_mutable_param", m_iParamIndex );
	m_iNumComponents = pKV->GetInt( "i_vp_mutable_nc", m_iNumComponents );

	char tmp[MAX_PATH];
	Q_snprintf( tmp, sizeof(tmp), "%s", pKV->GetString( "szConstantString" ) );
	m_iChannelInfo.Tokenize( tmp );
	m_iChannelInfo.PrintTargetString( m_szConstantString, MAX_PATH );

	UpdateNode();
}
bool CNodeVmtParam_Mutable::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	SetAllocating( false );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	tg->MakeConstantOnly();

	CHLSL_Solver_VParam_Mutable *solver = new CHLSL_Solver_VParam_Mutable( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->SetState( m_iParamIndex, m_iNumComponents );
	solver->SetComponentInfo( m_iChannelInfo );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}






CNodeVmtParam_Static::CNodeVmtParam_Static( CNodeView *p ) : BaseClass( "VParam static", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	m_iNumComponents = 3;
	m_szName = NULL;
	m_iChannelInfo.SetDefaultChannels();
	m_iChannelInfo.bAllowChars = false;
	m_szConstantString[0] = '\0';

	UpdateNode();
}
CNodeVmtParam_Static::~CNodeVmtParam_Static()
{
	delete [] m_szName;
}
void CNodeVmtParam_Static::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_Out( CBaseNode::BY_INDEX, m_hRestoreBridges );

	const bool bValid = m_szName && Q_strlen( m_szName );
	GenerateJacks_Output( bValid ? 1 : 0 );

	if ( bValid )
	{
		LockJackOutput_Flags( 0, ::GetVarTypeFlag( m_iNumComponents ), VarArgs( "%s", m_szName ) );
	}

	RestoreBridgesFromList_Out( m_hRestoreBridges );
}
KeyValues *CNodeVmtParam_Static::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetString( "i_vp_static_name", m_szName );
	pKV->SetInt( "i_vp_static_nc", m_iNumComponents );
	pKV->SetString( "szConstantString", m_szConstantString );

	return pKV;
}
void CNodeVmtParam_Static::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	const char *name = pKV->GetString( "i_vp_static_name" );
	if ( name && Q_strlen( name ) )
	{
		delete [] m_szName;
		m_szName = new char[ Q_strlen( name ) + 1 ];
		Q_strcpy( m_szName, name );
	}
	m_iNumComponents = pKV->GetInt( "i_vp_static_nc", m_iNumComponents );

	char tmp[MAX_PATH];
	Q_snprintf( tmp, sizeof(tmp), "%s", pKV->GetString( "szConstantString" ) );
	m_iChannelInfo.Tokenize( tmp );
	m_iChannelInfo.PrintTargetString( m_szConstantString, MAX_PATH );

	UpdateNode();
	OnUpdateHierachy( NULL, NULL );
}
bool CNodeVmtParam_Static::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( !m_szName || !Q_strlen( m_szName ) )
		return false;

	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	SetAllocating( false );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	tg->MakeConstantOnly();

	CHLSL_Solver_VParam_Static *solver = new CHLSL_Solver_VParam_Static( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->SetState( m_szName, m_iNumComponents );
	solver->SetComponentInfo( m_iChannelInfo );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}
