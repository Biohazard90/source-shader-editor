
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_Fog::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *var_params = GetSourceVar( 0 );
	CHLSL_Var *var_view_z = GetSourceVar( 1 );
	CHLSL_Var *var_world_z = GetSourceVar( 2 );

#ifdef SHADER_EDITOR_DLL_SWARM
	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = CalcPixelFogFactor( PIXELFOGTYPE, %s, %s, %s );\n",
		tg->GetName(),var_params->GetName(),var_view_z->GetName(),var_world_z->GetName() );
#else
	CHLSL_Var *var_proj_z = GetSourceVar( 3 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = CalcPixelFogFactor( PIXELFOGTYPE, %s, %s, %s, %s );\n",
		tg->GetName(),var_params->GetName(),var_view_z->GetName(),var_world_z->GetName(),var_proj_z->GetName() );
#endif

	context.buf_code.PutString( tmp );
}

void CHLSL_Solver_Fog::Render( Preview2DContext &c )
{
	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( 0, 0, 0, 0 );
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_Solver_Fog::OnIdentifierAlloc( IdentifierLists_t &List )
{
	List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_PIXELFOG ) );
}