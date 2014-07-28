
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_Lerp::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );
	CHLSL_Var *src2 = GetSourceVar( 1 );
	CHLSL_Var *src3 = GetSourceVar( 2 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = lerp( %s, %s, %s );\n",
		tg->GetName(),src1->GetName(),src2->GetName(),src3->GetName() );
	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_Lerp::Render( Preview2DContext &c )
{
	int type0 = GetSourceVar(0)->GetType();
	int type1 = GetSourceVar(1)->GetType();
	int type2 = GetSourceVar(2)->GetType();

	SetUVParamBySourceVar( NPSOP_CALC_LERP, 0, 0 );
	SetUVParamBySourceVar( NPSOP_CALC_LERP, 1, 1 );
	SetUVParamBySourceVar( NPSOP_CALC_LERP, 2, 2 );

	IMaterialVar *var_3 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_LERP, 3 );
	IMaterialVar *var_4 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_LERP, 4 );
	IMaterialVar *var_5 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_LERP, 5 );
	var_3->SetVecValue( 0, 0, 0, 0 );
	var_4->SetVecValue( 0, 0, 0, 0 );
	var_5->SetVecValue( 0, 0, 0, 0 );

	int maxSlots = ::GetSlotsFromTypeFlag( max( type0, type1 ) );
	if ( type0 != type1 )
	{
		Assert( maxSlots > 1 );
		bool target_is_0 = ( type0 < type1 );

		IMaterialVar *var_target = target_is_0 ? var_3 : var_4;
		var_target->SetVecValue( maxSlots >= 2, maxSlots >= 3, maxSlots >= 4, 0 );
	}
	if ( type2 == HLSLVAR_FLOAT1 )
		var_5->SetVecValue( maxSlots >= 2, maxSlots >= 3, maxSlots >= 4, 0 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_LERP ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}
