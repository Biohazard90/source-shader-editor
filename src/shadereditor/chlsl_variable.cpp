
#include "cbase.h"
#include "editorCommon.h"



CHLSL_Var::CHLSL_Var()
{
	m_tType = HLSLVAR_FLOAT1;

	m_bCanBeOverwritten = true;
	m_bCanBeDeclared = true;
	m_bGotDeclared = false;
	bCustomName = false;
	_varName = NULL;
	pOwner = NULL;

	m_iRTMapIndex = 0;

	m_pTex = NULL;
	m_pMat = NULL;

	AllocCheck_Alloc();
}

CHLSL_Var::CHLSL_Var( const CHLSL_Var& other )
{
	m_tType = other.m_tType;
	m_iRTMapIndex = 0;

	m_bCanBeOverwritten = other.m_bCanBeOverwritten;
	m_bCanBeDeclared = other.m_bCanBeDeclared;
	m_bGotDeclared = other.m_bGotDeclared;
	bCustomName = other.bCustomName;
	pOwner = NULL;

	//if ( m_tType == HLSLVAR_PP_RT )
	//	m_pTex = other.m_pTex;
	//else if ( m_tType == HLSLVAR_PP_MATERIAL )
	//	m_pMat = other.m_pMat;
	//else
	{
		m_pTex = NULL;
		m_pMat = NULL;
	}

	_varName = NULL;
	if ( other._varName != NULL )
	{
		int len = Q_strlen(other._varName) + 2;
		_varName = new char[len];
		Q_snprintf(_varName,len,"%s",other._varName);
	}

	AllocCheck_Alloc();
}
CHLSL_Var::CHLSL_Var( HLSLVariableTypes type )
{
	m_bCanBeOverwritten = true;
	m_bCanBeDeclared = true;
	m_bGotDeclared = false;
	bCustomName = false;
	_varName = NULL;

	m_iRTMapIndex = 0;

	pOwner = NULL;
	m_tType = type;

	m_pTex = NULL;
	m_pMat = NULL;

	AllocCheck_Alloc();
}
CHLSL_Var::~CHLSL_Var()
{
	if ( _varName )
		delete [] _varName;

	AllocCheck_Free();
}
void CHLSL_Var::Init()
{
	m_tType = HLSLVAR_FLOAT1;

	m_bCanBeOverwritten = true;
	m_bCanBeDeclared = true;
	m_bGotDeclared = false;
	bCustomName = false;
	_varName = NULL;

	m_iRTMapIndex = 0;
}

IMaterial *CHLSL_Var::GetMaterial()
{
	Assert( m_tType == HLSLVAR_PP_MATERIAL );
	//Assert( m_pMat );

	return m_pMat;
}
ITexture *CHLSL_Var::GetTexture()
{
	Assert( m_tType == HLSLVAR_PP_RT );
	//Assert( m_pTex );

	return m_pTex;
}
void CHLSL_Var::SetMaterial( IMaterial *pMat )
{
	Assert( m_tType == HLSLVAR_PP_MATERIAL );

	m_pMat = pMat;
}
void CHLSL_Var::SetTexture( ITexture *pTex )
{
	Assert( m_tType == HLSLVAR_PP_RT );

	m_pTex = pTex;
}

const bool &CHLSL_Var::WasDeclared()
{
	return m_bGotDeclared;
}
const void CHLSL_Var::OnDeclare()
{
	m_bGotDeclared = true;
}
void CHLSL_Var::DeclareMe( WriteContext_FXC &context, bool init )
{
	if ( WasDeclared() )
		return;
	char tmp[MAXTARGC];
	const char *codename = GetVarCodeNameFromFlag(GetType());
	Q_snprintf( tmp, MAXTARGC, "%s ", codename );
	context.buf_code.PutString( tmp );
	if ( init )
	{
		Q_snprintf( tmp, MAXTARGC, "%s = (%s)0;\n", GetName(), codename );
		context.buf_code.PutString( tmp );
	}
	OnDeclare();
}

const bool &CHLSL_Var::CanBeOverwritten()
{
	return m_bCanBeOverwritten;
}
const void CHLSL_Var::MakeConstantOnly()
{
	m_bCanBeOverwritten = false;
	m_bCanBeDeclared = false;
}
const void CHLSL_Var::ResetVarInfo()
{
	m_bGotDeclared = false;
}
CHLSL_SolverBase *CHLSL_Var::GetOwner()
{
	return pOwner;
}
void CHLSL_Var::SetOwner( CHLSL_SolverBase *n )
{
	pOwner = n;
}
void CHLSL_Var::Cleanup()
{
}
void CHLSL_Var::SetName( const char *name, bool Custom )
{
	delete [] _varName;
	int len = Q_strlen( name ) + 2;
	_varName = new char[ len ];
	Q_snprintf( _varName, len, "%s", name );

	bCustomName = Custom;
}
const char *CHLSL_Var::GetName()
{
	return _varName;
}
const bool CHLSL_Var::HasCustomName()
{
	return bCustomName;
}
