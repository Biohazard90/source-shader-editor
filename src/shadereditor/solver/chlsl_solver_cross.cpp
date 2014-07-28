
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_Cross::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );
	CHLSL_Var *src2 = GetSourceVar( 1 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = cross( %s, %s );\n",
		tg->GetName(),src1->GetName(),src2->GetName() );
	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_Cross::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_CALC_CROSS, 0, 0 );
	SetUVParamBySourceVar( NPSOP_CALC_CROSS, 1, 1 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_CROSS ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}