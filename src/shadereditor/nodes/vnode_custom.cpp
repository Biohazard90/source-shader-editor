
#include "cbase.h"
#include "editorCommon.h"

CNodeCustom::CNodeCustom( CNodeView *p ) : BaseClass( "Custom code", p )
{
	m_szFunctionName[0] = 0;
	m_szFilePath[0] = 0;

	m_pCode_Global = NULL;
	m_pCode_Function = NULL;

	m_bInline = true;
	m_iEnvFlags = 0;

	UpdateNode();
}

CNodeCustom::~CNodeCustom()
{
	delete [] m_pCode_Global;
	delete [] m_pCode_Function;

	PurgeNameVectors();
}

void CNodeCustom::PurgeNameVectors()
{
	m_hszVarNames_In.PurgeAndDeleteElements();
	m_hszVarNames_Out.PurgeAndDeleteElements();
}

char *CNodeCustom::GenerateDefaultName( int idx, bool bOut )
{
	const char *ref = bOut ? "out_%02i" : "in_%02i";
	static char tmp[32];

	Q_snprintf( tmp, sizeof(tmp), ref, idx );
	return tmp;

	//char *out = new char[ Q_strlen( tmp ) ];
	//Q_strcpy( out, tmp );
	//return out;
}

KeyValues *CNodeCustom::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	WriteJackDataFromKV( pKV, m_hszVarNames_In, m_hszVarNames_Out,
						GetNumJacks_In(), GetNumJacks_Out() );

	pKV->SetString( "szFunctionName", m_szFunctionName );
	pKV->SetString( "szFilePath", m_szFilePath );
	pKV->SetInt( "iInline", m_bInline ? 1 : 0 );

	CKVPacker::KVPack( m_pCode_Global, "szcode_global", pKV );
	CKVPacker::KVPack( m_pCode_Function, "szcode_body", pKV );
	//pKV->SetString( "szcode_global", m_pCode_Global );
	//pKV->SetString( "szcode_body", m_pCode_Function );
	pKV->SetInt( "iEnvFlags", m_iEnvFlags );

	return pKV;
}

void CNodeCustom::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	RestoreInternal( pKV );

	if ( !IsUsingInlineCode() )
		UpdateFromFile();

	UpdateNode();
}

void CNodeCustom::RestoreInternal( KeyValues *pKV )
{
	PurgeNameVectors();

	FillJackDataFromKV( pKV, m_hszVarNames_In, m_hszVarNames_Out );

	Q_snprintf( m_szFunctionName, sizeof(m_szFunctionName), "%s", pKV->GetString( "szFunctionName" ) );
	Q_snprintf( m_szFilePath, sizeof(m_szFilePath), "%s", pKV->GetString( "szFilePath" ) );
	m_bInline = !!pKV->GetInt( "iInline" );
	m_iEnvFlags = pKV->GetInt( "iEnvFlags" );

	delete [] m_pCode_Global;
	delete [] m_pCode_Function;
	m_pCode_Global = NULL;
	m_pCode_Function = NULL;

	m_pCode_Global = CKVPacker::KVUnpack( pKV, "szcode_global" );
	m_pCode_Function = CKVPacker::KVUnpack( pKV, "szcode_body" );
}

void CNodeCustom::UpdateFromFile()
{
	if ( IsUsingInlineCode() )
		return;

	if ( Q_strlen( GetFilePath() ) < 1 )
		return;

	//g_pFullFileSystem->RelativePathToFullPath( GetFilePath
	KeyValues *pData = new KeyValues("");
	pData->LoadFromFile( g_pFullFileSystem, GetFilePath(), "MOD" );
	RestoreInternal( pData );
	pData->deleteThis();
}

const char *CNodeCustom::GetFilePath()
{
	return m_szFilePath;
}

bool CNodeCustom::IsUsingInlineCode()
{
	return m_bInline;
}

void CNodeCustom::WriteJackDataFromKV( KeyValues *pKV, CUtlVector< __funcParamSetup* > &m_hIn, CUtlVector< __funcParamSetup* > &m_hOut,
										int maxIn, int maxOut )
{
	for ( int i = 0; i < maxIn; i++ )
	{
		const char *pName = VarArgs( "varName_In_%i", i );
		if ( i < m_hIn.Count() )
			pKV->SetString( pName, m_hIn[ i ]->pszName );
		else
			pKV->SetString( pName, "" );

		pName = VarArgs( "varType_In_%i", i );
		if ( i < m_hIn.Count() )
			pKV->SetInt( pName, m_hIn[ i ]->iFlag );
		else
			pKV->SetInt( pName, HLSLVAR_FLOAT1 );
	}

	for ( int i = 0; i < maxOut; i++ )
	{
		const char *pName = VarArgs( "varName_Out_%i", i );
		if ( i < m_hOut.Count() )
			pKV->SetString( pName, m_hOut[ i ]->pszName );
		else
			pKV->SetString( pName, "" );

		pName = VarArgs( "varType_Out_%i", i );
		if ( i < m_hOut.Count() )
			pKV->SetInt( pName, m_hOut[ i ]->iFlag );
		else
			pKV->SetInt( pName, HLSLVAR_FLOAT1 );
	}
}
void CNodeCustom::FillJackDataFromKV( KeyValues *pKV, CUtlVector< __funcParamSetup* > &m_hIn, CUtlVector< __funcParamSetup* > &m_hOut )
{
	Assert( m_hIn.Count() == 0 );
	Assert( m_hOut.Count() == 0 );

	const char *nullString = "";
	int itr = 0;

	for (;;)
	{
		const char *read_name = pKV->GetString( VarArgs( "varName_In_%i", itr ), nullString );
		const int read_type = pKV->GetInt( VarArgs( "varType_In_%i", itr ), 0 );

		if ( read_type == 0 )
			break;

		m_hIn.AddToTail( AllocOutputSetup( false, read_name, read_type ) );
		itr++;
	}

	itr = 0;

	for (;;)
	{
		const char *read_name = pKV->GetString( VarArgs( "varName_Out_%i", itr ), nullString );
		const int read_type = pKV->GetInt( VarArgs( "varType_Out_%i", itr ), 0 );

		if ( read_type == 0 )
			break;

		m_hOut.AddToTail( AllocOutputSetup( true, read_name, read_type ) );
		itr++;
	}
}
__funcParamSetup *CNodeCustom::AllocOutputSetup( bool bOutput, const char *name, int type )
{
	__funcParamSetup *p = new __funcParamSetup();
	AutoCopyStringPtr( name, &p->pszName );
	p->iFlag = type;
	p->bOutput = bOutput;

	char *pszFixSpaces = p->pszName;
	while ( pszFixSpaces && *pszFixSpaces )
	{
		if ( *pszFixSpaces == ' ' )
			*pszFixSpaces = '_';
		pszFixSpaces++;
	}

	return p;
}

void CNodeCustom::UpdateNode()
{
	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges_In;
	CreateBridgeRestoreData_In( CBaseNode::BY_INDEX, m_hRestoreBridges_In );
	GenerateJacks_Input( m_hszVarNames_In.Count() );
	//GenerateJacks_Input( numJacksIn );


	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges_Out;
	CreateBridgeRestoreData_Out( CBaseNode::BY_INDEX, m_hRestoreBridges_Out );
	GenerateJacks_Output( m_hszVarNames_Out.Count() );


	for ( int i = 0; i < GetNumJacks_In(); i++ )
	{
		char *name = NULL;

		if ( m_hszVarNames_In.Count() > i && m_hszVarNames_In[ i ]->pszName != NULL &&
			Q_strlen( m_hszVarNames_In[ i ]->pszName ) > 0 )
			name = m_hszVarNames_In[ i ]->pszName;
		else
			name = GenerateDefaultName( i );

		LockJackInput_Flags( i, m_hszVarNames_In[ i ]->iFlag, name );
	}

	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		char *name = NULL;

		if ( m_hszVarNames_Out.Count() > i && m_hszVarNames_Out[ i ]->pszName != NULL &&
			Q_strlen( m_hszVarNames_Out[ i ]->pszName ) > 0 )
			name = m_hszVarNames_Out[ i ]->pszName;
		else
			name = GenerateDefaultName( i, true );

		LockJackOutput_Flags( i, m_hszVarNames_Out[ i ]->iFlag, name );
	}

	RestoreBridgesFromList_In( m_hRestoreBridges_In );
	RestoreBridgesFromList_Out( m_hRestoreBridges_Out );


	OnUpdateHierachy( NULL, NULL );
}

void CNodeCustom::PreSolverUpdated()
{
	//UpdateNode();
}

int CNodeCustom::UpdateInputsValid()
{
	int base = BaseClass::UpdateInputsValid();
	int local = (/*(m_pCode_Global && Q_strlen(m_pCode_Global)) &&*/
		(m_pCode_Function && Q_strlen(m_pCode_Function)) &&
		(m_bInline || Q_strlen(m_szFilePath))) ? ERRORLEVEL_NONE : ERRORLEVEL_UNDEFINED;

	return max( base, local );
}

bool CNodeCustom::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	Assert( m_pCode_Function && Q_strlen( m_pCode_Function ) );
	Assert( m_bInline || Q_strlen(m_szFilePath) );

	SetAllocating( true );

	char *pszFuncString = CKVPacker::ConvertKVSafeString( m_pCode_Function, false );
	char *pszGlobalString = CKVPacker::ConvertKVSafeString( m_pCode_Global, false );

	int iHierachyType = GetHierachyTypeIterateFullyRecursive();
	int envflags = m_iEnvFlags;
	if ( iHierachyType & HLSLHIERACHY_PS )
		envflags &= ~NODECUSTOM_ADDENV_LIGHTING_VS;

	if ( iHierachyType & HLSLHIERACHY_VS )
		envflags &= ~NODECUSTOM_ADDENV_LIGHTING_PS;

	CHLSL_Solver_Custom *solver = new CHLSL_Solver_Custom( GetUniqueIndex() );
	solver->SetResourceType( RESOURCETYPE_VARIABLE );
	solver->Init( pszGlobalString, pszFuncString, envflags,
		m_szFunctionName, m_szFilePath,
		m_hszVarNames_In, m_hszVarNames_Out,
		this );

	delete [] pszFuncString;
	delete [] pszGlobalString;

	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In( i )->GetTemporaryVarTarget_End() );

	for ( int i = 0; i < GetNumJacks_Out(); i++ )
	{
		CJack *pJ = GetJack_Out( i );
		CHLSL_Var *pTg = pJ->AllocateVarFromSmartType();
		solver->AddTargetVar( pTg );
	}

	AddSolver( solver );

	return true;
}