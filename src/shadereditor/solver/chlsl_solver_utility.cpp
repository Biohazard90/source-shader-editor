
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_Utility_Declare::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = %s;\n",
		tg->GetName(),src1->GetName() );

	context.buf_code.PutString( tmp );
}
void CHLSL_Solver_Utility_Declare::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_UTILITY_DECLARE, 0, 0 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_UTILITY_DECLARE ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}



void CHLSL_Solver_Utility_Assign::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	CHLSL_Var *src1 = GetSourceVar( 1 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = %s;\n",
		tg->GetName(),src1->GetName() );

	context.buf_code.PutString( tmp );
}
void CHLSL_Solver_Utility_Assign::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_UTILITY_ASSIGN, 0, 0 );
	SetUVParamBySourceVar( NPSOP_UTILITY_ASSIGN, 1, 1 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_UTILITY_ASSIGN ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}
