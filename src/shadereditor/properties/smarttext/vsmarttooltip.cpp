
#include "cbase.h"
#include "vSmartObject.h"
#include "vSmartTooltip.h"
#include "vSmartText.h"


CSmartTooltip::CSmartTooltip( Panel *parent, const char *pElementname ) : BaseClass( parent, pElementname )
{
	m_iParamHighlight = 0;
	m_iMode = STTIPMODE_NORMAL;
	m_pCurObject = NULL;
	m_iFont = GetFontCacheHandle()->GetTooltipFont(); //GetFont( GetFontCacheHandle()->fSizeMin() + GetFontCacheHandle()->fStepSize() * 2 );

	SetVisible( true );
	MakePopup( false );

	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );
}

CSmartTooltip::~CSmartTooltip()
{
	delete m_pCurObject;
}

bool CSmartTooltip::IsFunctionParams()
{
	return m_iMode == STTIPMODE_FUNCTIONPARAMS;
}

void CSmartTooltip::SetActiveParam( int iP )
{
	m_iParamHighlight = iP;
}

bool CSmartTooltip::Init( CSmartObject *pO, int iMode )
{
	Assert( pO != NULL );
	m_iMode = iMode;

	delete m_pCurObject;
	m_pCurObject = new CSmartObject( *pO );

	int width = 0;
	int rows = 0;

	if ( m_iMode == STTIPMODE_FUNCTIONPARAMS )
	{
		width = surface()->GetCharacterWidth( m_iFont, ' ' ) * ( Q_strlen( pO->m_pszDefinition ) + 1 );

		int screenx, screeny;
		surface()->GetScreenSize( screenx, screeny );

		rows = 1;
		rows += width / screenx;

		width = min( screenx, width );
	}
	else
	{
		rows += GetNumRowsForString( pO->m_pszDefinition, &width );
		int crows = GetNumRowsForString( pO->m_pszHelptext, &width );
		if ( crows ) crows++;
		rows += crows;
		crows = GetNumRowsForString( pO->m_pszSourceFile, &width );
		if ( crows ) crows++;
		rows += crows;
	}


	if ( !rows || !width )
		return false;

	SetSize( width, rows * surface()->GetFontTall( m_iFont ) );

	return true;
}

void CSmartTooltip::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBorder( pScheme->GetBorder("ButtonBorder") );

	SetBgColor( Color( 150, 170, 220, 160 ) );
}

void CSmartTooltip::Paint()
{
	BaseClass::Paint();

	if ( !m_pCurObject )
		return;

	const char *pStr[] = {
		m_pCurObject->m_pszDefinition,
		m_pCurObject->m_pszHelptext,
		m_pCurObject->m_pszSourceFile,
	};

	int iFontTall = surface()->GetFontTall( m_iFont );
	int iCWidth = surface()->GetCharacterWidth( m_iFont, ' ' );
	int screenx, screeny;
	surface()->GetScreenSize( screenx, screeny );

	int cX, cY;
	int sX, sY;
	GetSize( sX, sY );
	sX -= iCWidth;

	const bool bFuncDraw = m_iMode == STTIPMODE_FUNCTIONPARAMS;

	Color colDef( 10, 10, 11, 255 );
	Color colHighlight( 0, 0, 0, 255 );

	if ( bFuncDraw )
		colDef = Color( 70, 70, 70, 255 );

	surface()->DrawSetTextFont( m_iFont );
	surface()->DrawSetTextColor( colDef );

	cY = 0;

	int iCurParam = 1;
	bool bHitBracket = false;

	for ( int i = 0; i < 3; i++ )
	{
		if ( !pStr[i] || !*pStr[i] )
			continue;

		cX = 0;

		const char *pWalk = pStr[i];
		bool bLastLinebreak = false;
		while ( *pWalk )
		{
			const bool bLineBreak = *pWalk == '\n';
			const bool bTab = *pWalk == '\t';
			const bool bComma = *pWalk == ',';
			const bool bBracket = *pWalk == '(';

			surface()->DrawSetTextPos( cX, cY );

			if ( bFuncDraw && bHitBracket )
				surface()->DrawSetTextColor( (!bComma && iCurParam == m_iParamHighlight) ? colHighlight : colDef );

			if ( !bLineBreak )
				surface()->DrawUnicodeChar( bTab ? L' ' : *pWalk );

			if ( bFuncDraw )
			{
				if ( bComma )
					iCurParam++;
				if ( bBracket )
					bHitBracket = true;
			}

			int curWidth = iCWidth; //surface()->GetCharacterWidth( m_iFont, *pWalk );

			if ( cX + (curWidth) < sX && !bLineBreak )
			{
				cX += curWidth;
				bLastLinebreak = false;
			}
			else if ( *( pWalk + 1 ) || bLineBreak && !bLastLinebreak )
			{
				cX = 0;
				cY += iFontTall;
				bLastLinebreak = bLineBreak;
			}

			pWalk++;
		}

		cY += iFontTall * 2;
	}
}

int CSmartTooltip::GetNumRowsForString( const char *pStr, int *max_x )
{
	if ( !pStr || !*pStr )
		return 0;

	int numRows = 1;

	//int x,y;
	//GetSize( x, y );
	const int x = 400;

	int curX = 0;
	int curMax = 0;

	int iCWidth = surface()->GetCharacterWidth( m_iFont, ' ' );

	const char *pWalk = pStr;
	bool bLastLinebreak = false;
	while ( pWalk && *pWalk )
	{
		const int curSize = iCWidth; //surface()->GetCharacterWidth( m_iFont, *pWalk );
		const bool bLineBreak = *pWalk == '\n';
		if ( curX + (curSize) < x && !bLineBreak )
		{
			curX += curSize;
			bLastLinebreak = false;
		}
		else if ( *( pWalk + 1 ) || bLineBreak && !bLastLinebreak )
		{
			curX = 0;
			numRows++;
			bLastLinebreak = bLineBreak;
		}

		curMax = max( curMax, curX );
		pWalk++;
	}

	if ( max_x != NULL )
		*max_x = max( *max_x, curMax + iCWidth * 2 );

	return numRows;
}