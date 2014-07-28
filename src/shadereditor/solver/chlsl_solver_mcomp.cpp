
#include "cbase.h"
#include "editorcommon.h"

void CHLSL_Solver_MComp::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	char tmp[MAXTARGC];

	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);

	Q_snprintf( tmp, MAXTARGC, "%s = { ", tg->GetName() );
	context.buf_code.PutString( tmp );

	if ( !bColumns )
	{
		int maxvars = GetNumSourceVars();
		for ( int i = 0; i < maxvars; i++ )
		{
			CHLSL_Var *varCur = GetSourceVar( i );
			Q_snprintf( tmp, MAXTARGC, "%s", varCur->GetName() );
			if ( i < (maxvars-1) )
				Q_strcat( tmp, ",\n\t\t\t", MAXTARGC );
			context.buf_code.PutString( tmp );
		}
	}
	else
	{
		int numColumns = 3;
		int numRows = 3;
		if ( iTargetMatrix >= HLSLVAR_MATRIX4X3 )
			numColumns = 4;
		if ( iTargetMatrix >= HLSLVAR_MATRIX4X4 )
			numRows = 4;

		for ( int r = 0; r < numRows; r++ )
		{
			for ( int c = 0; c < numColumns; c++ )
			{
				CHLSL_Var *varCur = GetSourceVar( c );
				Q_snprintf( tmp, MAXTARGC, "%s.%c", varCur->GetName(), GetCharFromChannelNum(r) );
				if ( r < (numRows-1) || c < (numColumns-1) )
					Q_strcat( tmp, ",", MAXTARGC );
				context.buf_code.PutString( tmp );
			}
			if ( r < (numRows-1) )
				context.buf_code.PutString( "\n\t\t\t" );
		}
	}


	Q_snprintf( tmp, MAXTARGC, " };\n" );
	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_MComp::SetState( int mtype, bool doC )
{
	iTargetMatrix = mtype;
	bColumns = doC;
}

void CHLSL_Solver_MComp::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_CALC_MCOMP, 0, 0 );
	SetUVParamBySourceVar( NPSOP_CALC_MCOMP, 1, 1 );
	SetUVParamBySourceVar( NPSOP_CALC_MCOMP, 2, 2 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_MCOMP ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}