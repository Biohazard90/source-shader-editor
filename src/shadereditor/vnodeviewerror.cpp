
#include "cbase.h"
#include "editorCommon.h"

static const char *szErrorNames[] = {
	"YOU ARE USING MULTIPLE TEXTURE NODES WITH THE SAME VMT PARAMETER (%s) BUT DIFFERENT TEXTURE REFERENCES!",
	"A RENDER VIEW NODE WITH 'DURING SCENE' CHECKED SHOULD BE AT THE VERY BEGINNING OF THE GRAPH!",
#ifdef SHADER_EDITOR_DLL_2006
	"USING UNSUPPORTED NODE IN SOURCE 2006 (%s)!",
#endif
};

#define ERROR_COLOR Color( 170, 64, 32, 255 ) //Color( 160, 24, 0, 255 )
#define ERROR_OUTLINE_SIZE 2
#define ERROR_DISTANCE 10

CNodeViewError::CNodeViewError( Panel *parent, int errorType ) : BaseClass( parent )
{
	AddActionSignalTarget( parent );

	Assert( ARRAYSIZE( szErrorNames ) == NVERROR_COUNT );

	m_iErrorType = errorType;
	m_bClosing = false;

	m_iTallCur = 1;
	m_iWide = 200;
	SetSize( m_iWide, m_iTallCur );

	pL = new Label( this, "", "" );

	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );

	m_iTallGoal = 10;
}

CNodeViewError::~CNodeViewError()
{
}

void CNodeViewError::SetText( const char *szParam )
{
	const char *szErrorText = szErrorNames[m_iErrorType];
	char tmp[MAX_PATH];
	if ( szParam != NULL )
		Q_snprintf( tmp, sizeof(tmp), szErrorText, szParam );
	else
		Q_snprintf( tmp, sizeof(tmp), szErrorText );

	pL->SetText( tmp );

	pL->SetWrap( true );
	pL->SetTextInset( 5, 5 );
	pL->SetContentAlignment( Label::a_northwest );
	pL->SetSize( m_iWide, m_iTallCur );
	pL->InvalidateLayout( true, true );
	pL->SetFgColor( ERROR_COLOR );
	pL->SizeToContents();

	int w;
	pL->GetContentSize( w, m_iTallMax );
	m_iTallMax += 5;

	InvalidateLayout( true );
	DoOpen();
}

const int CNodeViewError::GetErrorType()
{
	return m_iErrorType;
}

const bool CNodeViewError::IsAnimating()
{
	return m_iTallCur != m_iTallGoal;
}

const bool CNodeViewError::IsClosing()
{
	return m_bClosing;
}

void CNodeViewError::DoOpen()
{
	m_bClosing = false;
	m_iTallGoal = m_iTallMax;
}

void CNodeViewError::DoClose()
{
	m_bClosing = true;
	m_iTallGoal = 0;
}

void CNodeViewError::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( IsAnimating() )
	{
		float move = (m_iTallGoal - m_iTallCur) * gpGlobals->frametime * 10.0f;
		int dir = (move>0)?1:-1;
		move = max( 1, min( abs(m_iTallGoal-m_iTallCur), abs(move) ) ) * dir;
		m_iTallCur += move;
	}
	else if ( IsClosing() )
		PostActionSignal( new KeyValues( "DestroyUserError" ) );

	SetSize( m_iWide, m_iTallCur );

	if ( !GetParent() )
		return;

	int px, py;
	CNodeView *p = assert_cast< CNodeView* >( GetParent() );
	p->GetSize( px, py );

	int posx = px - m_iWide - ERROR_DISTANCE;
	int posy = py;

	for ( int i = p->GetNumUserErrorLabels() - 1; i >= 0; i-- )
	{
		CNodeViewError *pL = p->GetUserErrorLabel( i );
		if ( pL == this )
			break;

		int lx, ly;
		pL->GetSize( lx, ly );

		posy -= ly;
		posy -= ERROR_DISTANCE;
	}

	posy -= ERROR_DISTANCE + m_iTallCur;

	SetPos( posx, posy );
}

void CNodeViewError::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	pL->SetFgColor( ERROR_COLOR );
	SetBgColor( Color( 0, 0, 0, 128 ) );

	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );
}

void CNodeViewError::Paint()
{
	BaseClass::Paint();

	int sx, sy;
	GetSize( sx, sy );

	Color c = ERROR_COLOR;
	c[3] = 128 + 127 * abs(sin(gpGlobals->curtime * 3));

	surface()->DrawSetColor( c );
	surface()->DrawFilledRect( 0, 0, sx, ERROR_OUTLINE_SIZE );
	surface()->DrawFilledRect( 0, sy - ERROR_OUTLINE_SIZE, sx, sy );
	surface()->DrawFilledRect( 0, ERROR_OUTLINE_SIZE, ERROR_OUTLINE_SIZE, sy - ERROR_OUTLINE_SIZE );
	surface()->DrawFilledRect( sx - ERROR_OUTLINE_SIZE, ERROR_OUTLINE_SIZE, sx, sy - ERROR_OUTLINE_SIZE );
}