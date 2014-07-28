
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_VectorReflect::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );
	CHLSL_Var *src2 = GetSourceVar( 1 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = reflect( %s, %s );\n",
		tg->GetName(),src1->GetName(),src2->GetName() );
	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_VectorReflect::Render( Preview2DContext &c )
{
	int type0 = GetSourceVar(0)->GetType();
	int type1 = GetSourceVar(1)->GetType();

	SetUVParamBySourceVar( NPSOP_CALC_REFLECT, 0, 0 );
	SetUVParamBySourceVar( NPSOP_CALC_REFLECT, 1, 1 );

	IMaterialVar *var_2 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_REFLECT, 2 );
	IMaterialVar *var_3 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_REFLECT, 3 );
	var_2->SetVecValue( 0, 0, 0, 0 );
	var_3->SetVecValue( 0, 0, 0, 0 );

	if ( type0 != type1 )
	{
		int maxSlots = ::GetSlotsFromTypeFlag( max( type0, type1 ) );
		Assert( maxSlots > 1 );
		bool target_is_0 = ( type0 < type1 );

		IMaterialVar *var_target = target_is_0 ? var_2 : var_3;
		var_target->SetVecValue( maxSlots >= 2, maxSlots >= 3, maxSlots >= 4, 0 );
	}

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_REFLECT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}




void CHLSL_Solver_VectorRefract::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );
	CHLSL_Var *src2 = GetSourceVar( 1 );
	CHLSL_Var *src3 = GetSourceVar( 2 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = refract( %s, %s, %s );\n",
		tg->GetName(),src1->GetName(),src2->GetName(),src3->GetName() );
	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_VectorRefract::Render( Preview2DContext &c )
{
	int type0 = GetSourceVar(0)->GetType();
	int type1 = GetSourceVar(1)->GetType();

	SetUVParamBySourceVar( NPSOP_CALC_REFRACT, 0, 0 );
	SetUVParamBySourceVar( NPSOP_CALC_REFRACT, 1, 1 );
	SetUVParamBySourceVar( NPSOP_CALC_REFRACT, 2, 2 );

	IMaterialVar *var_2 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_REFRACT, 3 );
	IMaterialVar *var_3 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_REFRACT, 4 );
	var_2->SetVecValue( 0, 0, 0, 0 );
	var_3->SetVecValue( 0, 0, 0, 0 );

	if ( type0 != type1 )
	{
		int maxSlots = ::GetSlotsFromTypeFlag( max( type0, type1 ) );
		Assert( maxSlots > 1 );
		bool target_is_0 = ( type0 < type1 );

		IMaterialVar *var_target = target_is_0 ? var_2 : var_3;
		var_target->SetVecValue( maxSlots >= 2, maxSlots >= 3, maxSlots >= 4, 0 );
	}

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_REFRACT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}