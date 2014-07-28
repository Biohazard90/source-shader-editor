
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_TexTransform::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );

	char targetname[MAX_PATH];
	char srcname_uv[MAX_PATH];
	char srcname_center[MAX_PATH];
	char srcname_rot[MAX_PATH];
	char srcname_scale[MAX_PATH];
	char srcname_trans[MAX_PATH];

	Q_snprintf( targetname, MAX_PATH, "%s", tg->GetName() );

	char tmp[MAXTARGC];
	if ( !tg->WasDeclared() )
	{
		Q_snprintf( tmp, MAXTARGC, "%s %s;\n", GetVarCodeNameFromFlag(tg->GetType()), targetname );
		context.buf_code.PutString( tmp );
		tg->OnDeclare();
	}
	context.buf_code.PutString( "{\n" );
	context.buf_code.PushTab();

	int curIn = 0;
	Q_snprintf( srcname_uv, MAX_PATH, "%s", GetSourceVar( curIn )->GetName() );
	curIn++;

	if ( bEnable_Center )
	{
		Q_snprintf( srcname_center, MAX_PATH, "%s", GetSourceVar( curIn )->GetName() );
		curIn++;
	}
	else
	{
		Q_snprintf( srcname_center, MAX_PATH, "center_def" );
		context.buf_code.PutString( "float2 center_def = float2( 0.5f, 0.5f );\n" );
	}

	if ( bEnable_Rot )
	{
		Q_snprintf( srcname_rot, MAX_PATH, "%s", GetSourceVar( curIn )->GetName() );
		curIn++;
	}
	else
	{
		Q_snprintf( srcname_rot, MAX_PATH, "rot_def" );
		context.buf_code.PutString( "float rot_def = float( 0.0f );\n" );
	}

	if ( bEnable_Scale )
	{
		Q_snprintf( srcname_scale, MAX_PATH, "%s", GetSourceVar( curIn )->GetName() );
		curIn++;
	}
	else
	{
		Q_snprintf( srcname_scale, MAX_PATH, "scale_def" );
		context.buf_code.PutString( "float2 scale_def = float2( 1.0f, 1.0f );\n" );
	}

	if ( bEnable_Trans )
	{
		Q_snprintf( srcname_trans, MAX_PATH, "%s", GetSourceVar( curIn )->GetName() );
		curIn++;
	}
	else
	{
		Q_snprintf( srcname_trans, MAX_PATH, "trans_def" );
		context.buf_code.PutString( "float2 trans_def = float2( 0.0f, 0.0f );\n" );
	}

	Q_snprintf( tmp, MAXTARGC, "float fs = sin( %s );\n", srcname_rot );
	context.buf_code.PutString( tmp );
	Q_snprintf( tmp, MAXTARGC, "float fc = cos( %s );\n", srcname_rot );
	context.buf_code.PutString( tmp );

	Q_snprintf( tmp, MAXTARGC, "float4 row_0 = float4( fc * %s.x, -fs * %s.x, 0, ( -%s.x * fc + %s.y * fs ) * %s.x + %s.x + %s.x );\n",
		srcname_scale,srcname_scale,
		srcname_center,srcname_center,srcname_scale,srcname_center,srcname_trans );
	context.buf_code.PutString( tmp );
	Q_snprintf( tmp, MAXTARGC, "float4 row_1 = float4( fs * %s.y, fc * %s.y, 0, ( -%s.x * fs - %s.y * fc ) * %s.y + %s.y + %s.y );\n",
		srcname_scale,srcname_scale,
		srcname_center,srcname_center,srcname_scale,srcname_center,srcname_trans );
	context.buf_code.PutString( tmp );

	Q_snprintf( tmp, MAXTARGC, "%s.x = dot( float4( %s, 0, 1 ), row_0 );\n", targetname, srcname_uv );
	context.buf_code.PutString( tmp );
	Q_snprintf( tmp, MAXTARGC, "%s.y = dot( float4( %s, 0, 1 ), row_1 );\n", targetname, srcname_uv );
	context.buf_code.PutString( tmp );

	context.buf_code.PopTab();
	context.buf_code.PutString( "}\n" );
}

void CHLSL_Solver_TexTransform::Render( Preview2DContext &c )
{
	IMaterialVar *var_5 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_TEXTRANSFORM, 5 );

	Assert( GetNumSourceVars() <= 5 );
	int curSourceVar = 0;
	SetUVParamBySourceVar( NPSOP_CALC_TEXTRANSFORM, 0, curSourceVar ); // UV
	curSourceVar++;

	if ( bEnable_Center )
	{
		SetUVParamBySourceVar( NPSOP_CALC_TEXTRANSFORM, 1, curSourceVar );
		curSourceVar++;
	}
	if ( bEnable_Rot )
	{
		SetUVParamBySourceVar( NPSOP_CALC_TEXTRANSFORM, 2, curSourceVar );
		curSourceVar++;
	}
	if ( bEnable_Scale )
	{
		SetUVParamBySourceVar( NPSOP_CALC_TEXTRANSFORM, 3, curSourceVar );
		curSourceVar++;
	}
	if ( bEnable_Trans )
	{
		SetUVParamBySourceVar( NPSOP_CALC_TEXTRANSFORM, 4, curSourceVar );
		curSourceVar++;
	}

	var_5->SetVecValue( bEnable_Center, bEnable_Rot, bEnable_Scale, bEnable_Trans );
	
	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_TEXTRANSFORM ) );
	UpdateTargetVarToReflectMapIndex( 0 );

	//int type0 = GetSourceVar(0)->GetType();
	//int type1 = GetSourceVar(1)->GetType();

	//SetUVParamBySourceVar( NPSOP_CALC_ADD, 0, 0 );
	//SetUVParamBySourceVar( NPSOP_CALC_ADD, 1, 1 );

	//IMaterialVar *var_2 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_ADD, 2 );
	//IMaterialVar *var_3 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_ADD, 3 );
	//var_2->SetVecValue( 0, 0, 0, 0 );
	//var_3->SetVecValue( 0, 0, 0, 0 );

	//if ( type0 != type1 )
	//{
	//	int maxSlots = ::GetSlotsFromTypeFlag( max( type0, type1 ) );
	//	Assert( maxSlots > 1 );
	//	bool target_is_0 = ( type0 < type1 );

	//	IMaterialVar *var_target = target_is_0 ? var_2 : var_3;
	//	var_target->SetVecValue( maxSlots >= 2, maxSlots >= 3, maxSlots >= 4, 0 );
	//}

	//CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_ADD ) );
	//UpdateTargetVarToReflectMapIndex( 0 );
}