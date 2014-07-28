
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_Callback::CHLSL_Solver_Callback( const CHLSL_Solver_Callback &o ) : CHLSL_SolverBase( o )
{
	iCallbackIndex = o.iCallbackIndex;
	iNumComps = o.iNumComps;
	szName = NULL;
	if ( o.szName && Q_strlen( o.szName ) )
	{
		szName = new char[ Q_strlen( o.szName ) + 1 ];
		Q_strcpy( szName, o.szName );
	}
}

void CHLSL_Solver_Callback::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];

	Q_snprintf( tmp, MAXTARGC, "g_cData_%s", szName );

	target->SetName( tmp, true );
}

void CHLSL_Solver_Callback::SetState( int index, const char *name, int numComps )
{
	Assert( name && Q_strlen( name ) );
	iCallbackIndex = index;
	iNumComps = numComps;

	szName = new char[ Q_strlen( name ) + 1 ];
	Q_strcpy( szName, name );
}

void CHLSL_Solver_Callback::Render( Preview2DContext &c )
{
	float info_0[4] = { 0, 0, 0, 0 };
	_clCallback *cb = shaderEdit->GetCallback( iCallbackIndex );
	cb->func( info_0 );

	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( info_0, 4 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_Solver_Callback::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_SMART_CALLBACK;
	ec->iHLSLRegister = -1;
	ec->szSmartHelper = new char[ Q_strlen( szName ) + 1 ];
	ec->iSmartNumComps = iNumComps - 1;
	Q_strcpy( ec->szSmartHelper, szName );
	List.hList_EConstants.AddToTail( ec );
}