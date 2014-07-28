
#include "cbase.h"
#include "editorCommon.h"


CNodeMatrix_MVP::CNodeMatrix_MVP( CNodeView *p ) : BaseClass( "Matrix MVP", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_MATRIX4X4, "Model View Projection" );
}
bool CNodeMatrix_MVP::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	SetAllocating( false );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_MVP *solver = new CHLSL_Solver_MVP( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	AddSolver( solver );
	return true;
}


CNodeMatrix_VP::CNodeMatrix_VP( CNodeView *p ) : BaseClass( "Matrix VP", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_MATRIX4X4, "View Projection" );
}
bool CNodeMatrix_VP::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	SetAllocating( false );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_VP *solver = new CHLSL_Solver_VP( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	AddSolver( solver );
	return true;
}



CNodeMatrix_M::CNodeMatrix_M( CNodeView *p ) : BaseClass( "Matrix Model", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_MATRIX4X3, "Model transform" );
}
bool CNodeMatrix_M::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	SetAllocating( false );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_M *solver = new CHLSL_Solver_M( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	AddSolver( solver );
	return true;
}






CNodeMatrix_VM::CNodeMatrix_VM( CNodeView *p ) : BaseClass( "Matrix VP", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_MATRIX4X4, "View Model" );
}
bool CNodeMatrix_VM::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	SetAllocating( false );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_VM *solver = new CHLSL_Solver_VM( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	AddSolver( solver );
	return true;
}





CNodeMatrix_FVP::CNodeMatrix_FVP( CNodeView *p ) : BaseClass( "Flashlight VP", p )
{
	GenerateJacks_Output( 1 );
	LockJackOutput_Flags( 0, HLSLVAR_MATRIX4X4, "Flashlight View Proj" );
}
bool CNodeMatrix_FVP::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	SetAllocating( false );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_FVP *solver = new CHLSL_Solver_FVP( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	AddSolver( solver );
	return true;
}



CNodeMatrix_Custom::CNodeMatrix_Custom( CNodeView *p ) : BaseClass( "Custom matrix", p )
{
	m_iCustomID = CMATRIX_VIEW;
	GenerateJacks_Output( 1 );
	UpdateNode();
}
void CNodeMatrix_Custom::UpdateNode()
{
	const customMatrix_t *data = GetCMatrixInfo( m_iCustomID );
	LockJackOutput_Flags( 0, data->iHLSLVarFlag, data->szCanvasName );
}
KeyValues *CNodeMatrix_Custom::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );
	pKV->SetInt( "i_c_matrix", m_iCustomID );
	return pKV;
}
void CNodeMatrix_Custom::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_iCustomID = pKV->GetInt( "i_c_matrix" );
	UpdateNode();
}
bool CNodeMatrix_Custom::CreateSolvers(GenericShaderData *ShaderData)
{
	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();

	SetAllocating( false );
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	pJ_Out->SetTemporaryVarTarget( tg );

	CHLSL_Solver_CMatrix *solver = new CHLSL_Solver_CMatrix( GetUniqueIndex() );
	solver->SetMatrixID( m_iCustomID );
	solver->SetResourceType( res );
	solver->AddTargetVar( tg );
	AddSolver( solver );
	return true;
}