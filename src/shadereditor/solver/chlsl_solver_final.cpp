
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_Final::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);

	CHLSL_Var *var_color = GetSourceVar( 0 );
	CHLSL_Var *var_fog = GetSourceVar( 1 );
	CHLSL_Var *var_projz = bWriteDepth ? GetSourceVar( 2 ) : NULL;

	const char *tonemapmode = GetCodeTonemapMacro( iTonemap );
	char depthwritecombo[MAXTARGC];
	if ( bWriteDepth )
		Q_snprintf( depthwritecombo, MAXTARGC, "true, %s", var_projz->GetName() );
	else
		Q_snprintf( depthwritecombo, MAXTARGC, "false, 1.0f" );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = FinalOutput( %s, %s, PIXELFOGTYPE, %s, %s );\n",
		tg->GetName(), var_color->GetName(), var_fog->GetName(), tonemapmode, depthwritecombo );

	context.buf_code.PutString( tmp );
}
void CHLSL_Solver_Final::SetState( int t, bool wd )
{
	iTonemap = t;
	bWriteDepth = wd;
}
void CHLSL_Solver_Final::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_UTILITY_ASSIGN, 1, 0 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_UTILITY_ASSIGN ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_Solver_Final::OnIdentifierAlloc( IdentifierLists_t &List )
{
	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_PIXELFOG ) );
	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_WATERFOG_TOALPHA ) );
}