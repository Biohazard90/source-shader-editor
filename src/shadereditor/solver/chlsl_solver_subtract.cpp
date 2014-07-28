
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_Subtract::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );
	CHLSL_Var *src2 = GetSourceVar( 1 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = %s - %s;\n",
		tg->GetName(),src1->GetName(),src2->GetName() );
	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_Subtract::Render( Preview2DContext &c )
{
	int type0 = GetSourceVar(0)->GetType();
	int type1 = GetSourceVar(1)->GetType();

	SetUVParamBySourceVar( NPSOP_CALC_SUBTRACT, 0, 0 );
	SetUVParamBySourceVar( NPSOP_CALC_SUBTRACT, 1, 1 );

	IMaterialVar *var_2 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_SUBTRACT, 2 );
	IMaterialVar *var_3 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_SUBTRACT, 3 );
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

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_SUBTRACT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}




void CHLSL_Solver_Invert::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = 1.0f - %s;\n",
		tg->GetName(),src1->GetName() );
	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_Invert::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_CALC_INVERT, 0, 0 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_INVERT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}