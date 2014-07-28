
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_MVP::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "cModelViewProj" );
	target->SetName( tmp, true );
}



void CHLSL_Solver_VP::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "cViewProj" );
	target->SetName( tmp, true );
}



void CHLSL_Solver_M::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "cModel[0]" );
	target->SetName( tmp, true );
}



void CHLSL_Solver_VM::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "cViewModel" );
	target->SetName( tmp, true );
}



void CHLSL_Solver_FVP::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "g_cFlashlightWorldToTexture" );
	target->SetName( tmp, true );
}
void CHLSL_Solver_FVP::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_FLASHLIGHT_VPMATRIX;
	ec->iHLSLRegister = -1;
	ec->iConstSize = 4;
	List.hList_EConstants.AddToTail( ec );
}


void CHLSL_Solver_CMatrix::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s", GetCMatrixInfo( m_iMatrixID )->szIdentifierName );
	target->SetName( tmp, true );
}
void CHLSL_Solver_CMatrix::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_CUSTOM_MATRIX;
	ec->iHLSLRegister = -1;
	ec->iConstSize = GetCMatrixInfo( m_iMatrixID )->iRegSize;
	ec->iSmartNumComps = m_iMatrixID; // sucks. watch out for this in UniquifyConstantList
	List.hList_EConstants.AddToTail( ec );
}