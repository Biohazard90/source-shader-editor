
#include "cbase.h"
#include "editorCommon.h"

void CHLSL_Solver_Break::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	context.buf_code.PutString( "break;\n" );
}



void CHLSL_Solver_Clip::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *pvar = GetSourceVar( 0 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, sizeof( tmp ), "clip( %s );\n", pvar->GetName() );

	context.buf_code.PutString( tmp );
}
void CHLSL_Solver_Clip::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_UTILITY_DECLARE, 0, 0 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_UTILITY_DECLARE ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}