
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_Custom::CHLSL_Solver_Custom( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx )
{
	m_pFunc = NULL;
	m_iEnvFlags = 0;
}

CHLSL_Solver_Custom::CHLSL_Solver_Custom( const CHLSL_Solver_Custom &o ) : CHLSL_SolverBase( o )
{
	m_pFunc = NULL;
	m_iEnvFlags = o.m_iEnvFlags;

	if ( o.m_pFunc != NULL )
		m_pFunc = new SimpleFunction( *o.m_pFunc );
}

CHLSL_Solver_Custom::~CHLSL_Solver_Custom()
{
	delete m_pFunc;
}

void CHLSL_Solver_Custom::Init( const char *pszCode_Global, const char *pszCode_Body, int iEnvFlags,
		const char *pszFunctionName, const char *pszFilePath,
		const CUtlVector< __funcParamSetup* > &hParamsIn, const CUtlVector< __funcParamSetup* > &hParamsOut,
		CBaseNode *pCaller )
{
	delete m_pFunc;
	m_pFunc = new SimpleFunction();

	m_iEnvFlags = iEnvFlags;

	if ( pszCode_Global && Q_strlen(pszCode_Global) )
		AutoCopyStringPtr( pszCode_Global, &m_pFunc->szCode_Global );

	if ( pszCode_Body && Q_strlen(pszCode_Body) )
		AutoCopyStringPtr( pszCode_Body, &m_pFunc->szCode_Body );

	int funcNameFallbackLen = 128;
	if ( pszFunctionName && Q_strlen(pszFunctionName) )
		AutoCopyStringPtr( pszFunctionName, &m_pFunc->szFuncName );
	else if ( pszFilePath && Q_strlen( pszFilePath ) )
	{
		m_pFunc->szFuncName = new char[ funcNameFallbackLen ];
		m_pFunc->szFuncName[0] = '\0';
		Q_FileBase( pszFilePath, m_pFunc->szFuncName, funcNameFallbackLen );

		if ( !Q_strlen(m_pFunc->szFuncName) )
			Q_snprintf( m_pFunc->szFuncName, funcNameFallbackLen, "functionformaterror" );
	}
	else
	{
		m_pFunc->szFuncName = new char[ funcNameFallbackLen ];
		BuildDefaultFunctionName( m_pFunc->szFuncName, funcNameFallbackLen, pCaller );
	}

	if ( pszFilePath && Q_strlen(pszFilePath) )
		AutoCopyStringPtr( pszFilePath, &m_pFunc->szFilePath );

	for ( int i = 0; i < hParamsIn.Count(); i++ )
	{
		__funcParamSetup *pFP = hParamsIn[i];
		Assert( !pFP->bOutput );
		m_pFunc->hParams.AddToTail( new __funcParamSetup( *pFP ) );
	}

	for ( int i = 0; i < hParamsOut.Count(); i++ )
	{
		__funcParamSetup *pFP = hParamsOut[i];
		Assert( pFP->bOutput );
		m_pFunc->hParams.AddToTail( new __funcParamSetup( *pFP ) );
	}
}

void CHLSL_Solver_Custom::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	Assert( m_pFunc );
	Assert( m_pFunc->szFuncName );

	char tmp[MAX_PATH*4];
	BuildUserFunctionName( m_pFunc->szFuncName, m_pFunc->hParams, tmp, sizeof( tmp ) );

	if ( m_pFunc->szCode_Global )
	{
		context.buf_functions_globals.PutString( m_pFunc->szCode_Global );
	}

	context.buf_functions_bodies.PutString( tmp );
	context.buf_functions_bodies.PutString( "\n{\n" );
	if ( m_pFunc->szCode_Body )
		context.buf_functions_bodies.PutString( m_pFunc->szCode_Body );
	context.buf_functions_bodies.PutString( "\n}\n" );


	for ( int i = 0; i < GetNumTargetVars(); i++ )
	{
		CHLSL_Var *tg = GetTargetVar( i );
		tg->DeclareMe( context, true );
	}

	Q_snprintf( tmp, sizeof( tmp ), "%s(", m_pFunc->szFuncName );
	context.buf_code.PutString( tmp );

	for ( int i = 0; i < GetNumSourceVars(); i++ )
	{
		CHLSL_Var *var = GetSourceVar( i );
		context.buf_code.PutString( " " );
		context.buf_code.PutString( var->GetName() );

		if ( GetNumTargetVars() || i < GetNumSourceVars() - 1 )
			context.buf_code.PutString( "," );
		else
			context.buf_code.PutString( " " );
	}

	for ( int i = 0; i < GetNumTargetVars(); i++ )
	{
		CHLSL_Var *var = GetTargetVar( i );
		context.buf_code.PutString( " " );
		context.buf_code.PutString( var->GetName() );

		if ( i < GetNumTargetVars() - 1 )
			context.buf_code.PutString( "," );
		else
			context.buf_code.PutString( " " );
	}

	context.buf_code.PutString( ");\n" );
}

void CHLSL_Solver_Custom::Render( Preview2DContext &c )
{
	float info_0[4] = { 1, 1, 1, 1 };
	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( info_0, 4 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_Solver_Custom::OnIdentifierAlloc( IdentifierLists_t &List )
{
	if ( m_pFunc )
		List.hList_Functions.AddToTail( new SimpleFunction( *m_pFunc ) );

	if ( m_iEnvFlags & NODECUSTOM_ADDENV_LIGHTING_PS )
	{
		SimpleEnvConstant *ec = new SimpleEnvConstant();
		ec->iEnvC_ID = HLSLENV_STUDIO_LIGHTING_PS;
		ec->iHLSLRegister = -1;
		List.hList_EConstants.AddToTail( ec );

		List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_NUM_LIGHTS ) );
	}
	else if ( m_iEnvFlags & NODECUSTOM_ADDENV_LIGHTING_VS )
	{
		SimpleEnvConstant *ec = new SimpleEnvConstant();
		ec->iEnvC_ID = HLSLENV_STUDIO_LIGHTING_VS;
		ec->iHLSLRegister = -1;
		List.hList_EConstants.AddToTail( ec );

		List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_LIGHT_STATIC ) );
		List.hList_Combos.AddToTail( AllocateComboDataByID( HLSLCOMBO_LIGHT_DYNAMIC ) );
	}
}