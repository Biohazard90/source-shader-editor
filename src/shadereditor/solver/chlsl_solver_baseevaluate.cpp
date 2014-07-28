
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_BaseEvaluate::CHLSL_Solver_BaseEvaluate( HNODE nodeidx, int numInputs ) : CHLSL_SolverBase( nodeidx )
{
	numIn = numInputs;
}

void CHLSL_Solver_BaseEvaluate::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	//CHLSL_Var *src1 = GetSourceVar( 0 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = %s( ",
		tg->GetName(),GetOperatorName() );

	for ( int i = 0; i < numIn; i++ )
	{
		CHLSL_Var *src = GetSourceVar( i );
		Q_strcat( tmp, src->GetName(), MAXTARGC );
		if ( i < (numIn-1) )
			Q_strcat( tmp, ", ", MAXTARGC );
	}

	context.buf_code.PutString( tmp );
	context.buf_code.PutString( " );\n" );
}

void CHLSL_Solver_BaseEvaluate::Render( Preview2DContext &c )
{
	Assert( numIn < 3 );
	int maxType = HLSLVAR_FLOAT1;

	for ( int i = 0; i < numIn; i++ )
	{
		SetUVParamBySourceVar( GetRenderOperator(), i, i);
		int curType = GetSourceVar( i )->GetType();
		maxType = max( maxType, curType );
	}

	for ( int i = 0; i < numIn; i++ )
	{
		int curType = GetSourceVar( i )->GetType();
		IMaterialVar *mvar_ = pEditorRoot->GetUVTargetParam( GetRenderOperator(), 2 + i );
		if ( curType < maxType )
		{
			Assert( curType == HLSLVAR_FLOAT1 );
			mvar_->SetVecValue( maxType >= HLSLVAR_FLOAT2, maxType >= HLSLVAR_FLOAT3, maxType >= HLSLVAR_FLOAT4, 0 );
		}
		else
			mvar_->SetVecValue( 0, 0, 0, 0 );
	}

	IMaterialVar *info5 = pEditorRoot->GetUVTargetParam( GetRenderOperator(), 5 );
	info5->SetVecValue( maxType >= HLSLVAR_FLOAT2, maxType >= HLSLVAR_FLOAT3, maxType >= HLSLVAR_FLOAT4, 0 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( GetRenderOperator() ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}


// src uv:
// 0 1

// controller:
// 2 3

// master lerp
// 5