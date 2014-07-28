
#include "cbase.h"
#include "editorCommon.h"


void CHLSL_Solver_ContainerBasic::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	//context.buf_code.PutString( "\n" );
	//context.buf_code.PutString( "///////////////////////////\n" );
	//context.buf_code.PutString( "// CONTAINER SOLVER HERE //\n" );
	//context.buf_code.PutString( "///////////////////////////\n" );
	//context.buf_code.PutString( "\n" );
}


void CHLSL_Solver_LoopIntro::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	Assert( m_iCondition >= 0 && m_iCondition <= 3 );

	CHLSL_Var *pVarItr = GetTargetVar( 0 );
	const char *varname = pVarItr->GetName();

	char tmp[ MAXTARGC ];
	Q_snprintf( tmp, MAXTARGC, "for ( int %s = %i; %s %s %i; %s%s )\n",
								varname, m_iValue_Start,
								varname, GetConditionCodeString( m_iCondition ), m_iValue_End,
								varname, (bDecrement?"--":"++")
								);

	context.buf_code.PutString( tmp );
	context.buf_code.PutString( "{\n" );
	context.buf_code.PushTab();
}

void CHLSL_Solver_Iterator::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
}
void CHLSL_Solver_Iterator::Render( Preview2DContext &c )
{
	IMaterialVar *pMatVar = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar->SetStringValue( "[0 0 0 0]" );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_Solver_LoopOutro::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	context.buf_code.PopTab();
	context.buf_code.PutString( "}\n" );
}
void CHLSL_Solver_LoopOutro::Render( Preview2DContext &c )
{
}

CHLSL_Solver_ComboIntro::CHLSL_Solver_ComboIntro( const CHLSL_Solver_ComboIntro &o ) : CHLSL_SolverBase( o )
{
	m_iCondition = o.m_iCondition;
	m_bStatic = o.m_bStatic;
	m_iValue = o.m_iValue;

	m_szComboName = NULL;
	if ( o.m_szComboName && Q_strlen( o.m_szComboName ) )
	{
		m_szComboName = new char[ Q_strlen( o.m_szComboName ) + 1 ];
		Q_strcpy( m_szComboName, o.m_szComboName );
	}
}
void CHLSL_Solver_ComboIntro::SetState( int cond, const char *name, int val, bool bstatic )
{
	Assert( name );

	m_iCondition = cond;
	m_bStatic = bstatic;
	delete [] m_szComboName;
	m_szComboName = new char[ Q_strlen( name ) + 1 ];
	Q_strcpy( m_szComboName, name );
	m_iValue = val;
}
void CHLSL_Solver_ComboIntro::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	char tmp[MAXTARGC];
	context.buf_code.EnableTabs( false );
	Q_snprintf( tmp, MAXTARGC, "#if ( %s %s %i )\n", m_szComboName, ::GetConditionCodeString( m_iCondition ), m_iValue );
	context.buf_code.EnableTabs( true );
	context.buf_code.PutString( tmp );
	context.buf_code.PushTab();
}
void CHLSL_Solver_ComboIntro::OnIdentifierAlloc( IdentifierLists_t &List )
{
	Assert( m_szComboName );
	SimpleCombo *c = new SimpleCombo();
	c->bStatic = m_bStatic;
	c->min = 0;
	c->max = m_iValue;
	c->name = new char[Q_strlen(m_szComboName) + 1];
	c->iComboType = HLSLCOMBO_CUSTOM;
	Q_strcpy( c->name, m_szComboName );

	UpdateComboDataByString( c, m_szComboName, false );
	List.hList_Combos.AddToTail( c );
}
void CHLSL_Solver_ComboOutro::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	context.buf_code.PopTab();
	context.buf_code.EnableTabs( false );
	context.buf_code.PutString( "#endif\n" );
	context.buf_code.EnableTabs( true );
}




void CHLSL_Solver_ConditionIntro::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	Assert( m_iCondition >= 0 && m_iCondition <= 3 );

	CHLSL_Var *pVar_0 = GetSourceVar( 0 );
	CHLSL_Var *pVar_1 = GetSourceVar( 1 );

	const char *varname_0 = pVar_0->GetName();
	const char *varname_1 = pVar_1->GetName();

	char tmp[ MAXTARGC ];
	Q_snprintf( tmp, MAXTARGC, "if ( %s %s %s )\n",
					varname_0, GetConditionCodeString( m_iCondition ), varname_1
					);

	context.buf_code.PutString( tmp );
	context.buf_code.PutString( "{\n" );
	context.buf_code.PushTab();
}