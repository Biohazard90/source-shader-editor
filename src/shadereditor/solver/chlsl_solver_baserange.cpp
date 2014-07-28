
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_BaseRange::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	Assert( 0 );
}

void CHLSL_Solver_BaseRange::Render( Preview2DContext &c )
{
	int OP = GetRenderOperator();
	int type0 = GetSourceVar(0)->GetType();
	int type1 = GetSourceVar(1)->GetType();
	int type2 = GetSourceVar(2)->GetType();

	SetUVParamBySourceVar( OP, 0, 0 );
	SetUVParamBySourceVar( OP, 1, 1 );
	SetUVParamBySourceVar( OP, 2, 2 );

	IMaterialVar *var_3 = pEditorRoot->GetUVTargetParam( OP, 3 );
	IMaterialVar *var_4 = pEditorRoot->GetUVTargetParam( OP, 4 );
	//IMaterialVar *var_5 = pEditorRoot->GetUVTargetParam( OP, 5 );
	var_3->SetVecValue( 0, 0, 0, 0 );
	var_4->SetVecValue( 0, 0, 0, 0 );
	//var_5->SetVecValue( 0, 0, 0, 0 );

	int maxSlots = ::GetSlotsFromTypeFlag( type2 );
	if ( type2 != type0 )
		var_3->SetVecValue( maxSlots >= 2, maxSlots >= 3, maxSlots >= 4, 0 );
	if ( type2 != type1 )
		var_4->SetVecValue( maxSlots >= 2, maxSlots >= 3, maxSlots >= 4, 0 );

	pEditorRoot->GetUVTargetParam( OP, 5 )->SetVecValue( maxSlots >= 2, maxSlots >= 3, maxSlots >= 4, 0 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( OP ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}



void CHLSL_Solver_Smoothstep::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );
	CHLSL_Var *src2 = GetSourceVar( 1 );
	CHLSL_Var *src3 = GetSourceVar( 2 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = smoothstep( %s, %s, %s );\n",
		tg->GetName(),src1->GetName(),src2->GetName(),src3->GetName() );
	context.buf_code.PutString( tmp );
}
void CHLSL_Solver_Clamp::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );
	CHLSL_Var *src2 = GetSourceVar( 1 );
	CHLSL_Var *src3 = GetSourceVar( 2 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = clamp( %s, %s, %s );\n",
		tg->GetName(),src3->GetName(),src1->GetName(),src2->GetName() );
	context.buf_code.PutString( tmp );
}