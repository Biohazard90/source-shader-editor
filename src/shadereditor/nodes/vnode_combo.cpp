
#include "cbase.h"
#include "editorcommon.h"


CNodeCombo::CNodeCombo( CNodeView *p ) : BaseClass( "Combo", p )
{
	m_iCondition = ITERATORCOND_EQUAL;
	m_iValue_Compare = 0;
	m_bStatic = false;

	const char *defname = "UNKNOWNCOMBO";
	m_szComboName = new char [Q_strlen( defname ) + 1];
	Q_strcpy( m_szComboName, defname );
}

CNodeCombo::~CNodeCombo()
{
	delete [] m_szComboName;
}

void CNodeCombo::Solve_ContainerEntered()
{
	CHLSL_Solver_ComboIntro *solver_intro = new CHLSL_Solver_ComboIntro( GetUniqueIndex() );
	solver_intro->SetState( m_iCondition, m_szComboName, m_iValue_Compare, m_bStatic );
	AddSolver( solver_intro );

	CHLSL_Solver_ContainerBasic *solver_dummy = new CHLSL_Solver_ContainerBasic( GetUniqueIndex() );
	AddSolver( solver_dummy );
}
void CNodeCombo::Solve_ContainerLeft()
{
	CHLSL_Solver_ComboOutro *solver_outro = new CHLSL_Solver_ComboOutro( GetUniqueIndex() );
	AddSolver( solver_outro );
}

KeyValues *CNodeCombo::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetInt( "i_combo_condition", m_iCondition );
	pKV->SetInt( "i_combo_value", m_iValue_Compare );
	pKV->SetInt( "i_combo_static", m_bStatic ? 1 : 0 );
	if ( Q_stricmp( "UNKNOWNCOMBO", m_szComboName ) )
		pKV->SetString( "sz_cname", m_szComboName );

	return pKV;
}
void CNodeCombo::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	BaseClass::RestoreFromKeyValues_Specific( pKV );

	m_iCondition = pKV->GetInt( "i_combo_condition", m_iCondition );
	m_iValue_Compare = pKV->GetInt( "i_combo_value", m_iValue_Compare );
	m_bStatic = !!pKV->GetInt( "i_combo_static", m_bStatic );
	const char *name = pKV->GetString( "sz_cname" );

	if ( name && Q_strlen( name ) )
	{
		delete [] m_szComboName;
		m_szComboName = new char[Q_strlen( name ) + 1];
		Q_strcpy( m_szComboName, name );
	}

	pNodeView->MakeSolversDirty();
}
bool CNodeCombo::VguiDraw( bool bShadow )
{
	if ( !BaseClass::VguiDraw( bShadow ) )
		return false;

	if ( !m_szComboName || !Q_strlen( m_szComboName ) )
		return false;

	bool bVis = false;
	HFont _font = pNodeView->GetFontScaled( 10.0f, bVis );
	if ( !bVis )
		return false;

	const Vector2D bmin = GetBoundsBoxMin();
	const Vector2D bmax = GetBoundsBoxMax();
	const float _offset = PREVIEWINSET + 0.5f;
	Vector2D text_pos( bmin.x + _offset, bmax.y + _offset + 10.0f );
	pNodeView->ToPanelSpace( text_pos );

	char szraw[MAX_PATH];
	wchar_t szconverted[ MAX_PATH ];

	Q_snprintf( szraw, MAX_PATH, "%s", m_szComboName );
	g_pVGuiLocalize->ConvertANSIToUnicode( szraw, szconverted, sizeof(szconverted)  );

	surface()->DrawSetTextFont( _font );
	surface()->DrawSetTextPos( text_pos.x, text_pos.y );

	surface()->DrawSetTextColor( NODE_DRAW_COLOR_TEXT_OP );
	surface()->DrawPrintText( szconverted, wcslen( szconverted ) );
	return true;
}