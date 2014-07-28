
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_Multiply::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	char tmp[MAXTARGC];
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);

	CHLSL_Var *src1 = GetSourceVar( 0 );
	CHLSL_Var *src2 = GetSourceVar( 1 );

	int t1 = src1->GetType();
	int t2 = src2->GetType();

	char szSecondSourceString[MAX_PATH];
	Q_snprintf( szSecondSourceString, MAX_PATH, "%s%s",
		( m_bMatrixRotationOnly ? "(float3x3)" : "" ),
		src2->GetName() );

	if ( t1 == HLSLVAR_FLOAT1 || t2 == HLSLVAR_FLOAT1 )
		Q_snprintf( tmp, MAXTARGC, "%s = %s * %s;\n", tg->GetName(), src1->GetName(), src2->GetName() );
	else if ( t1 == t2 )
	{
		if ( t1 >= HLSLVAR_MATRIX3X3 )
			Q_snprintf( tmp, MAXTARGC, "%s = mul( %s, %s );\n", tg->GetName(), src1->GetName(), szSecondSourceString );
		else
			Q_snprintf( tmp, MAXTARGC, "%s = %s * %s;\n", tg->GetName(), src1->GetName(), szSecondSourceString );
	}
	else
	{
		if ( t1 == HLSLVAR_FLOAT3 && t2 >= HLSLVAR_MATRIX4X3 )
			Q_snprintf( tmp, MAXTARGC, "%s = mul( float4(%s.xyz,1), %s );\n", tg->GetName(), src1->GetName(), szSecondSourceString );
		else
			Q_snprintf( tmp, MAXTARGC, "%s = mul( %s, %s );\n", tg->GetName(), src1->GetName(), szSecondSourceString );
	}

	context.buf_code.PutString( tmp );
}
void CHLSL_Solver_Multiply::Render( Preview2DContext &c )
{
	int type0 = GetSourceVar(0)->GetType();
	int type1 = GetSourceVar(1)->GetType();
	const bool bHasFloat1 = type0 == HLSLVAR_FLOAT1 || type1 == HLSLVAR_FLOAT1;

	SetUVParamBySourceVar( NPSOP_CALC_MULTIPLY, 0, 0 );
	SetUVParamBySourceVar( NPSOP_CALC_MULTIPLY, 1, 1 );

	IMaterialVar *var_2 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_MULTIPLY, 2 );
	IMaterialVar *var_3 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_MULTIPLY, 3 );
	var_2->SetVecValue( 0, 0, 0, 0 );
	var_3->SetVecValue( 0, 0, 0, 0 );

	if ( type0 != type1 && bHasFloat1 )
	{
		int maxSlots = ::GetSlotsFromTypeFlag( max( type0, type1 ) );
		Assert( maxSlots > 1 );
		bool target_is_0 = ( type0 < type1 );

		IMaterialVar *var_target = target_is_0 ? var_2 : var_3;
		var_target->SetVecValue( maxSlots >= 2, maxSlots >= 3, maxSlots >= 4, 0 );
	}

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_MULTIPLY ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}
