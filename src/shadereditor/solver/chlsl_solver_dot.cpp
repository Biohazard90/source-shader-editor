
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_Dot::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );
	CHLSL_Var *src2 = GetSourceVar( 1 );

	const bool bInitFloat1 = src1->GetType() != src2->GetType();
	const char *codename = ::GetVarCodeNameFromFlag( src2->GetType() );
	char enumElements[MAXTARGC];
	enumElements[ 0 ] = '\0';

	int enumFlag = ::GetSlotsFromTypeFlag( src2->GetType() );
	for ( int i = 0; i < enumFlag; i++ )
	{
		Q_strcat(enumElements,src1->GetName(),MAXTARGC);
		if ( i < (enumFlag-1) )
			Q_strcat(enumElements,", ",MAXTARGC);
	}

	char tmp[MAXTARGC];
	if ( bInitFloat1 )
		Q_snprintf( tmp, MAXTARGC, "%s = dot( %s( %s ), %s );\n",
			tg->GetName(), codename, enumElements, src2->GetName() );
	else
		Q_snprintf( tmp, MAXTARGC, "%s = dot( %s, %s );\n",
			tg->GetName(),src1->GetName(),src2->GetName() );
	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_Dot::Render( Preview2DContext &c )
{
	int type0 = GetSourceVar(0)->GetType();
	int type1 = GetSourceVar(1)->GetType();

	if ( type0 != type1 )
		pEditorRoot->GetUVTargetParam( NPSOP_CALC_DOT, 2 )->SetVecValue( type1 >= HLSLVAR_FLOAT2,
																		type1 >= HLSLVAR_FLOAT3,
																		type1 >= HLSLVAR_FLOAT4, 0 );
	else
		pEditorRoot->GetUVTargetParam( NPSOP_CALC_DOT, 2 )->SetVecValue( 0, 0, 0, 0 );

	SetUVParamBySourceVar( NPSOP_CALC_DOT, 0, 0);
	SetUVParamBySourceVar( NPSOP_CALC_DOT, 1, 1 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_DOT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}
