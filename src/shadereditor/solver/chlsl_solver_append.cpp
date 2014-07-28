
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_Append::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );
	CHLSL_Var *src2 = GetSourceVar( 1 );

	const char *varCodeName = ::GetVarCodeNameFromFlag( tg->GetType() );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = %s( %s, %s );\n",
		tg->GetName(), varCodeName,
		src1->GetName(), src2->GetName() );
	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_Append::Render( Preview2DContext &c )
{
	int numcomp_0 = ::GetSlotsFromTypeFlag( GetSourceVar(0)->GetType() );

	SetUVParamBySourceVar( NPSOP_CALC_APPEND, 0, 0 );
	SetUVParamBySourceVar( NPSOP_CALC_APPEND, 1, 1 );

	IMaterialVar *var_2 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_APPEND, 2 );
	var_2->SetVecValue( numcomp_0 == 1, numcomp_0 == 2, numcomp_0 == 3, numcomp_0 == 4 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_APPEND ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}