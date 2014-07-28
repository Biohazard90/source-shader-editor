
#include "cbase.h"
#include "editorCommon.h"



CHLSL_Solver_PP_Mat::CHLSL_Solver_PP_Mat( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx )
{
	m_bInline = true;
	m_pszInfoString = NULL;
	m_pszNodeName = NULL;
	InitMaterialInfo();
}
CHLSL_Solver_PP_Mat::~CHLSL_Solver_PP_Mat()
{
	DestroyMaterialInfo();
	delete [] m_pszInfoString;
	delete [] m_pszNodeName;
}
CHLSL_Solver_PP_Mat::CHLSL_Solver_PP_Mat( const CHLSL_Solver_PP_Mat &o ) : CHLSL_Solver_PP_Base( o )
{
	AutoCopyStringPtr( o.m_pszInfoString, &m_pszInfoString );
	AutoCopyStringPtr( o.m_pszNodeName, &m_pszNodeName );

	m_bInline = o.m_bInline;
	InitMaterialInfo();
}



void CHLSL_Solver_PP_Mat::Init( bool bInline, const char *pszPath, const char *pszScript, const char *pszUserName )
{
	Assert( m_pszInfoString == NULL );
	Assert( m_pszNodeName == NULL );
	Assert( !bInline || pszScript && Q_strlen( pszScript ) );
	Assert( bInline || pszPath && Q_strlen( pszPath ) );

	m_bInline = bInline;

	if ( m_bInline )
		AutoCopyStringPtr( pszScript, &m_pszInfoString );
	else
		AutoCopyStringPtr( pszPath, &m_pszInfoString );

	AutoCopyStringPtr( pszUserName, &m_pszNodeName );
}

void CHLSL_Solver_PP_Mat::OnExecuteCode( const RunCodeContext &context )
{
	Assert( m_pszInfoString );

	if ( !pMat )
	{
		if ( !m_bInline )
			pMat = materials->FindMaterial( m_pszInfoString, TEXTURE_GROUP_OTHER );
		else
		{
			Assert( !pKVMat );
			Assert( !m_pMatName );

			pKVMat = new KeyValues("");
			if ( !pKVMat->LoadFromBuffer( "", m_pszInfoString, g_pFullFileSystem ) )
			{
				Assert(0);
			}

			if ( pKVMat )
			{
				pKVMat->SetInt( "$__PPE_INLINE", 1 );

				char tmp[MAXTARGC];
				Q_snprintf( tmp, sizeof(tmp), "inline_material_%u", (unsigned int)GetData().iNodeIndex );

				int len = Q_strlen( tmp ) + 1;
				m_pMatName = new char[len];
				Q_snprintf( m_pMatName, len, "%s", tmp );

				pMat =  materials->CreateMaterial( tmp, pKVMat );
				pMat->IncrementReferenceCount();
			}
		}

		Assert( pMat );
	}

	CHLSL_Var *pT = GetTargetVar(0);

	if ( !pT->GetMaterial() )
		pT->SetMaterial( pMat );
}


void CHLSL_Solver_PP_Mat::InitMaterialInfo()
{
	pMat = NULL;
	pKVMat = NULL;
	m_pMatName = NULL;
}

void CHLSL_Solver_PP_Mat::DestroyMaterialInfo()
{
	Assert( !m_bInline || ( pMat == NULL ) == ( pKVMat == NULL ) );

	if ( m_bInline )
	{
		if ( pKVMat )
		{
			pKVMat->Clear();
			pKVMat = NULL;
		}

		if ( pMat )
		{
			//pMat->Release();
			//pMat->DeleteIfUnreferenced();
			ForceDeleteMaterial( &pMat );
		}
	}

	delete [] m_pMatName;
}

const char *CHLSL_Solver_PP_Mat::GetNodeName()
{
	return m_pszNodeName;
}

IMaterial *CHLSL_Solver_PP_Mat::GetMaterial()
{
	CHLSL_Var *pT = GetTargetVar(0);
	if ( pT )
		return pT->GetMaterial();
	return NULL;
}

const bool CHLSL_Solver_PP_Mat::IsInline()
{
	return m_bInline;
}

const char *CHLSL_Solver_PP_Mat::GetMaterialName()
{
	return m_pMatName;
}

KeyValues *CHLSL_Solver_PP_Mat::AccessKeyValues()
{
	return pKVMat;
	//if ( pKVMat )
	//	return pKVMat->MakeCopy();
	//return NULL;
}