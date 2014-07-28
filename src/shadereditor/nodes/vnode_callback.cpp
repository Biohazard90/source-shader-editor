
#include "cbase.h"
#include "editorCommon.h"


CNodeCallback::CNodeCallback( CNodeView *p ) : BaseClass( "Callback", p )
{
	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	szCallbackName = NULL;
	iNumComponents = 1;

	UpdateNode();
}

CNodeCallback::~CNodeCallback()
{
	delete [] szCallbackName;
}

void CNodeCallback::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_Out( CBaseNode::BY_INDEX, m_hRestoreBridges );

	if ( !szCallbackName )
	{
		GenerateJacks_Output( 0 );

		SaveDeleteVector( m_hRestoreBridges );
		return;
	}

	GenerateJacks_Output( 1 );
	int flag = ::GetVarTypeFlag( iNumComponents - 1 );
	LockJackOutput_Flags( 0, flag, szCallbackName );

	RestoreBridgesFromList_Out( m_hRestoreBridges );
}

KeyValues *CNodeCallback::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetString( "sz_callbackname", szCallbackName );
	pKV->SetInt( "i_numc", iNumComponents );
	return pKV;
}
void CNodeCallback::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	const char *cbname = pKV->GetString( "sz_callbackname" );

	if ( Q_strlen( cbname ) )
	{
		delete [] szCallbackName;
		szCallbackName = new char [ Q_strlen( cbname ) + 1 ];
		Q_strcpy( szCallbackName, cbname );
	}

	iNumComponents = pKV->GetInt( "i_numc", iNumComponents );

	UpdateNode();
	OnUpdateHierachy( NULL, NULL );
}

int CNodeCallback::UpdateInputsValid()
{
	if ( !szCallbackName || !Q_strlen( szCallbackName ) )
		return ERRORLEVEL_UNDEFINED;
	if ( shaderEdit->FindCallback( szCallbackName ) < 0 )
		return ERRORLEVEL_UNDEFINED;

	return BaseClass::UpdateInputsValid();
}

bool CNodeCallback::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( !szCallbackName || !Q_strlen( szCallbackName ) || !iNumComponents )
		return false;
	int target = shaderEdit->FindCallback( szCallbackName );
	if ( target < 0 )
		return false;

	CJack *pJ_Out = GetJack_Out( 0 );

	const int res = pJ_Out->GetResourceType();

	SetAllocating( true );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	tg->MakeConstantOnly();

	CHLSL_Solver_Callback *solver = new CHLSL_Solver_Callback( GetUniqueIndex() );
	solver->SetState( target, szCallbackName, iNumComponents );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	AddSolver( solver );

	return true;
}