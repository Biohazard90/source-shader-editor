
#include "cbase.h"
#include "editorCommon.h"

CNodePP_Mat::CNodePP_Mat( CNodeView *p ) : BaseClass( "Material", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_PP_MATERIAL, "Material" );

	m_szMatPath[0] = '\0';
	m_pszInlineVMT = NULL;
	m_bInline = true;

	OnUpdateHierachy( NULL, NULL );
}

CNodePP_Mat::~CNodePP_Mat()
{
	delete [] m_pszInlineVMT;
}

KeyValues *CNodePP_Mat::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetString( "szVMTPath", m_szMatPath );
	pKV->SetInt( "iInline", m_bInline ? 1 : 0 );

	if ( m_bInline && m_pszInlineVMT && Q_strlen(m_pszInlineVMT) > 0 )
	{
		CKVPacker::KVPack( m_pszInlineVMT, "szVMTString", pKV );
		//pKV->SetString( "szVMTString", m_pszInlineVMT );
	}
	
	return pKV;
}

void CNodePP_Mat::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	Q_snprintf( m_szMatPath, sizeof(m_szMatPath), "%s", pKV->GetString( "szVMTPath" ) );
	m_bInline = !!pKV->GetInt( "iInline" );

	delete [] m_pszInlineVMT;
	m_pszInlineVMT = NULL;
	//const char *pszInlineCode = ( m_bInline ? pKV->GetString( "szVMTString" ) : NULL );

	char *pszInlineCode = m_bInline ? CKVPacker::KVUnpack( pKV, "szVMTString" ) : NULL;

	if ( pszInlineCode && Q_strlen(pszInlineCode) > 0 )
		AutoCopyStringPtr( pszInlineCode, &m_pszInlineVMT );

	delete [] pszInlineCode;

	OnUpdateHierachy(NULL,NULL);
}

int CNodePP_Mat::UpdateInputsValid()
{
	if (m_bInline && (!m_pszInlineVMT || !*m_pszInlineVMT))
		return ERRORLEVEL_UNDEFINED;

	if (!m_bInline && (!*m_szMatPath || !g_pFullFileSystem->FileExists(m_szMatPath)))
		return ERRORLEVEL_UNDEFINED;

	return ERRORLEVEL_NONE;
}

bool CNodePP_Mat::CreateSolvers(GenericShaderData *ShaderData)
{
	const char *pszUserName = GetUserName();
	char *pUnsafeInline = CKVPacker::ConvertKVSafeString( m_pszInlineVMT, false );

	CHLSL_Var *pVar = GetJack_Out(0)->AllocateVarFromSmartType(); //new CHLSL_Var( HLSLVAR_PP_RT );

	CHLSL_Solver_PP_Mat *solver = new CHLSL_Solver_PP_Mat( GetUniqueIndex() );
	solver->AddTargetVar( pVar );
	solver->Init( m_bInline, m_szMatPath, pUnsafeInline, pszUserName );
	AddSolver( solver );

	delete [] pUnsafeInline;
	return true;
}