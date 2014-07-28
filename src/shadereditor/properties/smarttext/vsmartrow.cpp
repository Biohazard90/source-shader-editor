
#include "cbase.h"
#include "vSmartRow.h"
#include "cRegex.h"


#define __ROWCALC_INTRO( x ) int iLastsplit = 0;\
	int cur_x = 0;\
	int cWidth = 0;\
	for ( int i = 0; i < x; i++ )\
	{\
		wchar_t &ch = hText[ i ]

#define __ROWCALC_SPLIT_INTRO() if ( ShouldSplit( cur_x, iLastsplit, i ) )\
		{\
			iLastsplit = i;\
			cur_x = GetAutoCRWidth()

#define __ROWCALC_SPLIT_OUTRO() }\
	cWidth = GetCharWidth( ch, cur_x )

#define __ROWCALC_OUTRO() cur_x += cWidth;\
	}


_inlineFormat::_inlineFormat()
{
	start = 0;
	end = 1;

	bOverride_fg = false;
	bOverride_bg = false;

	bCRBlock = false;
}

bool _inlineFormat::ShouldOverride( int pos )
{
	return pos >= start && ( end == -1 || pos < end );
}

_cmtCtrl::_cmtCtrl()
{
	_cmtCtrl( 0, 0, 0 );
}
_cmtCtrl::_cmtCtrl( int row, int x0, int x1 )
{
	_y = row;
	_x0 = x0;
	_x1 = x1;
}

bool _cmtCtrl::IsCommented( int x, int y )
{
	return _y == y &&
		x >= _x0 &&
		( _x1 == -1 || x <= _x1 );
}

_smartRow::_smartRow( HFont font, const int &w, const int &inset )
{
	m_pszHighlightRef = NULL;

	_font = font;
	m_iInteriorWidth = w;
	m_iInsetWidth = inset;
	m_bDirty = false;
	SetLocked( false );
	SetDeletable( true );
	m_bDirty_HighlightRef = false;
	m_bDynamicSpacing_Enabled = false;

	m_iCached_Charwidth = 0;
	m_iCached_Renderlen = 0;
	MakeLayoutDirty();

	SetupVguiTex( m_iTex_Arrow, "shadereditor/char_arrow" );
	SetupVguiTex( m_iTex_CR, "shadereditor/char_cr" );
	
#if DEFAULT_ROW_CAPACITY > 0
	hText.EnsureCapacity( DEFAULT_ROW_CAPACITY );
#endif
#if DEFAULT_ROW_GROWSIZE > 0
	hText.SetGrowSize( DEFAULT_ROW_GROWSIZE );
#endif
}

_smartRow::~_smartRow()
{
	hText.Purge();
	hSyntaxFormat.PurgeAndDeleteElements();
	ClearHighlightFormats();
	ClearTopFormats();
}

_smartRow::_smartRow( const _smartRow &o )
{
	m_bDirty_HighlightRef = false;
	m_bDynamicSpacing_Enabled = o.m_bDynamicSpacing_Enabled;
	m_pszHighlightRef = NULL;

	_font = o._font;
	m_iInteriorWidth = o.m_iInteriorWidth;
	m_iInsetWidth = o.m_iInsetWidth;

	hText.AddVectorToTail( o.hText );

	for ( int i = 0; i < o.hSyntaxFormat.Count(); i++ )
		hSyntaxFormat.AddToTail( new _inlineFormat( *o.hSyntaxFormat[i] ) );
	for ( int i = 0; i < o.hUserHighlight.Count(); i++ )
		hUserHighlight.AddToTail( new _inlineFormat( *o.hUserHighlight[i] ) );
	for ( int i = 0; i < o.hTopHighlight.Count(); i++ )
		hTopHighlight.AddToTail( new _inlineFormat( *o.hTopHighlight[i] ) );

	m_iTex_Arrow = o.m_iTex_Arrow;
	m_iTex_CR = o.m_iTex_CR;

	m_iCached_Charwidth = o.m_iCached_Charwidth;
	m_iCached_Renderlen = o.m_iCached_Renderlen;
	m_bDirty = o.m_bDirty;
	m_bDirty_CharWidth = o.m_bDirty_CharWidth;
	m_bDirty_Renderlen = o.m_bDirty_Renderlen;

	m_bLocked = o.m_bLocked;
	m_bDeletable = o.m_bDeletable;
}


void _smartRow::SetLocked( bool bLocked )
{
	m_bLocked = bLocked;
}
bool _smartRow::IsLocked()
{
	return m_bLocked;
}
void _smartRow::SetDeletable( bool bDeletable )
{
	m_bDeletable = bDeletable;
}
bool _smartRow::IsDeletable()
{
	if ( IsLocked() )
		return false;

	return m_bDeletable;
}


void _smartRow::DoSyntaxHighlighting( const bool bStringLiterals, const bool bBrackets, const bool bHLSL )
{
	hSyntaxFormat.PurgeAndDeleteElements();

	//int LFs = 0;
	KillLineBreaks();

	for ( int x0 = 0; x0 < hText.Count(); x0++ )
	{
		//if ( !hText[ x0 ] )
		//{
		//	LFs++;
		//	continue;
		//}

		if ( IS_SPACE( hText[ x0 ] ) )
			continue;

		int x1 = GetWordRight( x0, true, true );

		if ( x1 == x0 )
			continue;

		wchar_t *word = ReadInterval( x0, x1 - 1, false );

		if ( word == NULL )
			continue;

		const Color *pCol = CRegex::GetColorForWordBySyntax( word, bStringLiterals,
								bBrackets, bHLSL );

		if ( pCol != NULL )
		{
			_inlineFormat *f = new _inlineFormat();
			f->bOverride_fg = true;
			f->col_fg = *pCol;
			f->start = x0; // - LFs;
			f->end = x1; // - LFs;

			hSyntaxFormat.AddToTail( f );
		}

		delete [] word;

		x0 = x1 - 1;
	}

	UpdateLinebreaks();
}

void _smartRow::SetFont( HFont f )
{
	_font = f;

	MakeLayoutDirty();
}

void _smartRow::SetInteriorWidth( const int &w, const int &inset )
{
	m_iInteriorWidth = w;
	m_iInsetWidth = inset;

	OnTextChanged( true );
}

int _smartRow::GetInteriorWidth()
{
	return m_iInteriorWidth;
}

void _smartRow::KillLineBreaks()
{
	for ( int i = 0; i < hText.Count(); i++ )
	{
		if ( !hText[i] )
		{
			hText.Remove( i );
			i--;
		}
	}
}
void _smartRow::UpdateLinebreaks()
{
#if DEBUG
	for ( int i = 0; i < hText.Count(); i++ )
		if ( !hText[i] )
			Assert( 0 );
#endif

	__ROWCALC_INTRO( hText.Count() );
	__ROWCALC_SPLIT_INTRO();
			hText.InsertBefore( i, L'\0' );
	__ROWCALC_SPLIT_OUTRO();
	__ROWCALC_OUTRO();
}

void _smartRow::SetHighlightRef( wchar_t *pszRef )
{
	m_pszHighlightRef = pszRef;

	if ( pszRef )
		MakeHighlightRefDirty();
}

void _smartRow::ClearHighlightFormats()
{
	m_pszHighlightRef = NULL;

	hUserHighlight.PurgeAndDeleteElements();
}

void _smartRow::AddHighlightFormat( _inlineFormat *format )
{
	hUserHighlight.AddToTail( format );
}

void _smartRow::ClearTopFormats()
{
	hTopHighlight.PurgeAndDeleteElements();
}

void _smartRow::AddTopFormat( _inlineFormat *format )
{
	hTopHighlight.AddToTail( format );
}

bool _smartRow::IsDynamicSpacingEnabled()
{
	return m_bDynamicSpacing_Enabled;
}

void _smartRow::SetDynamicSpacingEnabled( bool bEnabled )
{
	m_bDynamicSpacing_Enabled = bEnabled;
}

void _smartRow::DrawRow( const int &x, int &y,
						const Color &col_fg )
{
	if ( IsHighlightRefDirty() )
		MakeHighlightRefClean();

	int c_t = surface()->GetFontTall( _font );
	int RealPos = 0;

	CUtlVector< _inlineFormat* > hFormatStack;

	__ROWCALC_INTRO( hText.Count() );
	__ROWCALC_SPLIT_INTRO();
			y += c_t;

			int ico_tall = c_t / 2;
			surface()->DrawSetTexture( m_iTex_CR );
			surface()->DrawSetColor( ST_COL_SPACING );
			surface()->DrawTexturedRect( x + ico_tall, y + ico_tall / 2,
				x + ico_tall * 3, y + ico_tall / 2 + ico_tall );

	__ROWCALC_SPLIT_OUTRO();

		surface()->DrawSetTextColor( col_fg );

		for ( int f = 0; f < hFormatStack.Count(); f++ )
			if ( !hFormatStack[ f ]->ShouldOverride( RealPos ) )
			{
				hFormatStack.Remove( f );
				f--;
			}

		for ( int f = 0; f < hSyntaxFormat.Count(); f++ )
			if ( hSyntaxFormat[ f ]->ShouldOverride( RealPos ) && !hFormatStack.HasElement( hSyntaxFormat[ f ] ) )
				hFormatStack.AddToHead( hSyntaxFormat[ f ] );

		for ( int f = 0; f < hUserHighlight.Count(); f++ )
			if ( ( hUserHighlight[ f ]->ShouldOverride( RealPos ) ) && !hFormatStack.HasElement( hUserHighlight[ f ] ) )
				hFormatStack.AddToTail( hUserHighlight[ f ] );

		for ( int f = 0; f < hTopHighlight.Count(); f++ )
			if ( ( hTopHighlight[ f ]->ShouldOverride( RealPos ) ) && !hFormatStack.HasElement( hTopHighlight[ f ] ) )
				hFormatStack.AddToTail( hTopHighlight[ f ] );

		for ( int f = 0; f < hFormatStack.Count(); f++ )
		{
			_inlineFormat *format = hFormatStack[ f ];

			if ( format->bOverride_fg )
				surface()->DrawSetTextColor( format->col_fg );
			if ( format->bOverride_bg )
			{
				int rect_w = cWidth;
				if ( rect_w == 0 )
					rect_w = GetCharWidth( L' ' ) / 2;

				surface()->DrawSetColor( format->col_bg );
				surface()->DrawFilledRect( cur_x + x, y, cur_x + x + rect_w, y + c_t );
			}
		}

		if ( ch )
			RealPos++;

		surface()->DrawSetTextPos( cur_x + x, y );

		if ( !IsDynamicSpacingEnabled() )
		{
			if ( ch == ' ' )
			{
				surface()->DrawSetTextColor( ST_COL_SPACING );
				surface()->DrawUnicodeChar( L'\x00B7' );
			}
			else if ( ch == 9 )
			{
				int ico_tall = (int)(c_t*0.42f); // 42!
				int ico_spacing = ( c_t - ico_tall ) / 2;
				surface()->DrawSetTexture( m_iTex_Arrow );
				surface()->DrawSetColor( ST_COL_SPACING );
				surface()->DrawTexturedRect( cur_x + x, y + ico_spacing, cur_x + x + ico_tall * 2, y + ico_spacing + ico_tall );
			}
			else if ( ch != 0 )
			{
				surface()->DrawUnicodeChar( ch );
			}
		}

	__ROWCALC_OUTRO();

	for ( int f = 0; f < hUserHighlight.Count(); f++ )
	{
		_inlineFormat *format = hUserHighlight[ f ];

		if ( !format->bCRBlock )
			continue;

		if ( format->bOverride_fg )
			surface()->DrawSetTextColor( format->col_fg );
		if ( format->bOverride_bg )
		{
			int rect_w = GetCharWidth( L' ' ) / 2;

			surface()->DrawSetColor( format->col_bg );
			surface()->DrawFilledRect( cur_x + x, y, cur_x + x + rect_w, y + c_t );
		}
	}

	y += c_t;
	hFormatStack.Purge();
}

int _smartRow::GetRenderNumRows_Internal( int cursor_x, bool bPartial )
{
	int heightOut = 1;
	cursor_x = min( cursor_x, hText.Count() );

	__ROWCALC_INTRO( cursor_x );
		if ( !ch && bPartial )
		{
			cursor_x = min( cursor_x + 1, hText.Count() );
		}
	__ROWCALC_SPLIT_INTRO();
			heightOut++;
	__ROWCALC_SPLIT_OUTRO();
	__ROWCALC_OUTRO();

	return heightOut;
}


int _smartRow::GetRenderNumRows( int cursor_x )
{
	int result = 0;
	const bool bLimited = cursor_x >= 0;

	if ( bLimited || IsLayoutDirty_Renderlen() )
	{
		if ( !bLimited )
			cursor_x = hText.Count();

		result = GetRenderNumRows_Internal( cursor_x, bLimited );

		if ( !bLimited )
		{
			m_iCached_Renderlen = result;
			MakeLayoutClean_Renderlen();
		}
	}
	else
		result = m_iCached_Renderlen;

	return result;
}
int _smartRow::GetRenderHeight()
{
	return GetRenderNumRows() * surface()->GetFontTall( _font );
}
void _smartRow::RelativeRowCursorToPanel( const int &cursor_x, int &movex, int &movey )
{
	movey = 0;
	int c_t = surface()->GetFontTall( _font );

	__ROWCALC_INTRO( min( hText.Count(), cursor_x ) );
	__ROWCALC_SPLIT_INTRO();
			movey += c_t;
	__ROWCALC_SPLIT_OUTRO();
	__ROWCALC_OUTRO();

	movex = cur_x;
	movex += m_iInsetWidth;
}

int _smartRow::PanelToCursorCalcX( int virtualRow, int pixelx )
{
	pixelx -= m_iInsetWidth;

	if ( hText.Count() < 1 )
		return 0;

	int best_pos = 0;
	int best_dist = 99999;
	bool bGotHit = false;

	__ROWCALC_INTRO( hText.Count() );
		if ( virtualRow == 0 )
		{
			int cur_dist = abs( cur_x - pixelx );

			if ( cur_dist <= best_dist )
			{
				best_dist = cur_dist;
				best_pos = i;

				bGotHit = true;
			}
			else if ( bGotHit )
				return best_pos;
		}
	__ROWCALC_SPLIT_INTRO();
			virtualRow--;
			if ( virtualRow < 0 )
				return i;
	__ROWCALC_SPLIT_OUTRO();
	__ROWCALC_OUTRO();

	if ( abs( cur_x - pixelx ) < best_dist )
		best_pos++;

	return best_pos;
}

bool _smartRow::ShouldSplit( int curpixelpos, int localRowStart, int localRowCur )
{
	Assert( localRowStart < hText.Count() && localRowCur < hText.Count() );

	wchar_t &ch = hText[ localRowCur ];
	if ( !ch )
		return true;

	//int cwidth = ( ch == L'\t' ) ? GetCharWidth( L' ' ) : GetCharWidth( ch );
	int cwidth = GetCharWidth( ch, curpixelpos );

	if ( curpixelpos + cwidth > m_iInteriorWidth )
		return true;

	// is there a space to the left?
	//for ( int i = localRowStart; i < localRowCur; i++ )
	//	if ( IS_SPACE( hText[ i ] ) )
	if ( localRowCur > 0 && IS_SPACE( hText[ localRowCur - 1 ] ) )
		{
			int tmpPixelPos = curpixelpos;
			// any more spaces to the right?
			for ( int c = localRowCur; c < hText.Count(); c++ )
			{
				wchar_t &curWord = hText[ c ];
				if ( IS_SPACE( curWord ) )
					return false;
				tmpPixelPos += GetCharWidth( curWord, tmpPixelPos );
				if ( tmpPixelPos > m_iInteriorWidth )
					break;
			}

			tmpPixelPos = curpixelpos;
			// does this 'word' cross the border?
			for ( int c = localRowCur; c < hText.Count(); c++ )
			{
				wchar_t &curWord = hText[ c ];
				tmpPixelPos += GetCharWidth( curWord, tmpPixelPos );
				if ( tmpPixelPos > m_iInteriorWidth )
					return true;
			}
			//break;
		}

	return false;
}

int _smartRow::GetWordLeft( int pos )
{
	Assert( pos >= 0 && pos <= hText.Count() );

	if ( pos == hText.Count() )
		pos--;

	while ( pos >= 0 && hText[ pos ] == 0 )
		pos--;

	if ( pos < 0 )
		return 0;

	const wchar_t ref = hText[ pos ];
	wchar_t lastCh = ref;

	//bool bHitMinus = false;
	//bool bHitChar = false;

	pos--;

	for (;;)
	{
		if ( pos < 0 )
			return 0;

		const wchar_t cur = hText[ pos ];

		if ( !IsWordConcat( ref, cur, false, false ) )
		{
			bool bCur_Period = cur == L'.';
			bool bRef_Period = ref == L'.';

			if ( ( bRef_Period || bCur_Period ) )
			{
				int nextStart = pos + ( bCur_Period ? -1 : 0 );

				if ( nextStart >= 0 )
				{
					const wchar_t &ch = hText[ nextStart ];

					if ( IS_SPACE( ch ) )
						nextStart++;
					else
						nextStart = GetWordLeft( nextStart );
				}

				if ( nextStart >= 0 )
				{
					const wchar_t &ch = hText[ nextStart ];

					if ( IS_NUMERICAL( ch ) )
					{
						pos = nextStart - 1;
					}
				}
			}

			return max( 0, pos + 1 );
		}

		lastCh = cur;

		//bool bMinus = cur == L'-';
		//if ( IS_CHAR( cur ) )
		//	bHitChar = true;

		//if ( ( !bMinus || bHitMinus ) &&
		//	!IsWordConcat( ref, cur, false, !bHitChar ) )
		//	return max( 0, min( pos + 1, maxchar ) );

		//if ( bMinus )
		//	bHitMinus = true;

		pos--;
	}

	return 0;
}

int _smartRow::GetWordRight( int pos, bool bPreprocTest, bool bStringTest )
{
	Assert( pos >= 0 && pos <= hText.Count() );

	if ( pos == hText.Count() )
		pos--;

	int maxLen = hText.Count();
	int read0 = max( 0, pos );
	const wchar_t ref = hText[ read0 ];

	if ( bPreprocTest && ref == L'#' )
	{
		int end = -1;

		for ( int i = pos + 1; i < hText.Count() && end == -1; i++ )
		{
			if ( !hText[i] || !IS_SPACE( hText[i] ) )
				end = i;
		}

		if ( end >= 0 )
		{
			end = GetWordRight( end );

			wchar_t *strStart = ReadInterval( pos, end - 1 ); //hText.Base() + pos;

			if ( CRegex::MatchSyntax_Preproc( strStart ) )
			{
				for ( int x = pos+1; x < hText.Count(); x++ )
				{
					if ( hText[ x ] && !IS_SPACE( hText[ x ] ) )
						return GetWordRight( x );
				}
			}

			delete [] strStart;
		}
	}
	else if ( bStringTest &&
		( ref == L'\"' || ref == L'\'' ) )
	{
		for ( int i = read0 + 1; i < hText.Count(); i++ )
		{
			if ( hText[ i ] == '\\' )
			{
				i++;
				continue;
			}

			if ( hText[ i ] == ref )
				return i+1;
		}

		return maxLen;
	}

	pos++;

	for (;;)
	{
		if ( pos >= maxLen )
			return maxLen;

		const wchar_t cur = hText[ pos ];

		if ( !IsWordConcat( ref, cur, true, true ) )
			return pos;

		pos++;
	}

	return maxLen - 1;
}

bool _smartRow::IsWordConcat( const wchar_t ref, const wchar_t test, bool bLoose, bool bIsValue )
{
	if ( IS_SPACE( ref ) && IS_SPACE( test ) )
		return true;

	bool bLoose_ref = bLoose && ( ref == L'-' || ref == L'.' );
	//bool bLoose_test = bLoose && ( test == L'-' || test == L'.' );
	bool bLoose_test = bLoose && ( test == L'.' );
	//bool bLoose_ref = ref == L'-' || bLoose && ref == L'.';
	//bool bLoose_test = test == L'-' || bLoose && test == L'.';

	//if ( bIsValue &&
	//	( ref == L'.' || IS_NUMERICAL( ref ) || bLoose_ref ) )
	//{
	//	if ( IS_NUMERICAL( test ) || test == L'.' || bLoose_test )
	//		return true;

	//	if ( !bLoose && IS_CHAR( test ) )
	//		return true;
	//}

	if ( ref == L'.' ||
		ref == L';' ||
		ref == L':' ||
		ref == L'\'' ||
		ref == L'\"' )
		return false;

	if ( ( IS_NUMERICAL( ref ) || bLoose_ref ) &&
		( IS_NUMERICAL( test ) || bLoose_test ) )
		return true;

	if ( ( IS_CHAR( ref ) || IS_NUMERICAL( ref ) ) &&
		( IS_CHAR( test ) || IS_NUMERICAL( test ) ) )
		return true;

	if ( ref == test )
	{
		if ( ref == L'&' ||
			ref == L'|' ||
			ref == L'=' ||
			ref == L'+' ||
			ref == L'-' ) //&& abs(ref-test) <= 1 )
			return true;
	}

	return false;
}

int _smartRow::GetTextLen( bool bIgnoreLineBreaks )
{
	if ( !bIgnoreLineBreaks )
		return hText.Count();
	else
	{
		int c = 0;
		for ( int i = 0; i < hText.Count(); i++ )
			if ( hText[i] )
				c++;
		return c;
	}
}

void _smartRow::ToRealCursor( text_cursor &c )
{
	int cPos = ( c.x < hText.Count() && hText[ c.x ] != 0 ) ? c.x + 1 : c.x;
	for ( int i = 0; i < min( cPos, hText.Count() ); i++ )
	{
		if ( !hText[i] )
			c.x--;
	}
}

void _smartRow::ToVirtualCursor( text_cursor &c )
{
	for ( int i = 0; i < min( c.x, hText.Count() ); i++ )
	{
		if ( !hText[i] )
			c.x++;
	}
}

bool _smartRow::HasLinebreakAtCursor( const text_cursor &c )
{
	return hText.Count() > c.x && hText[ c.x ] == 0;
}

int _smartRow::GetCharWidth( wchar_t c, int rowPos )
{
	if ( !c )
		return 0;

	Assert( rowPos >= 0 );

	if ( IsLayoutDirty_CharWidth() )
	{
		m_iCached_Charwidth = surface()->GetCharacterWidth( _font, ' ' );
		MakeLayoutClean_CharWidth();
	}

	if ( c == L'\t' )
	{
		int pixelRight = m_iInteriorWidth - rowPos;
		pixelRight = clamp( pixelRight / m_iCached_Charwidth, 1, 4 );
		int dynSize = ( 4 - ( (rowPos/m_iCached_Charwidth) % 4 ) );

		if ( pixelRight < 4 )
			return m_iCached_Charwidth * min( pixelRight, dynSize );
		else
			return m_iCached_Charwidth * dynSize;
	}

	return m_iCached_Charwidth;
}

const int _smartRow::GetAutoCRWidth()
{
	return GetCharWidth( L' ' ) * 2;
}

int _smartRow::InsertChar( const wchar_t c, int pos, bool bFast, bool bSmartTabs )
{
	if ( IsLocked() )
		return 0;

	Assert( c != L'\r' && c != L'\n' );

	if ( pos < hText.Count() )
		hText.InsertBefore( pos, c );
	else
		hText.AddToTail( c );

	int curMoved = 0;

	if ( bSmartTabs && c == 9 )
	{
		//KillLineBreaks();
		int parserPos = clamp( pos - 1, 0, hText.Count() - 1 );

		while ( parserPos >= 0 &&
			( IS_SPACE( hText[ parserPos ] ) || !hText[ parserPos ] ) )
		{
			parserPos--;
		}

		parserPos++;
		ConvertSpaceToTab( parserPos );

		/*
		int numSpaces = 0;
		int parserPos = clamp( pos - 1, 0, hText.Count() - 1 );

		while ( parserPos >= 0 &&
			( IS_SPACE( hText[ parserPos ] ) || !hText[ parserPos ] ) )
		{
			if ( hText[ parserPos ] == ' ' )
			{
				hText.Remove( parserPos );
				numSpaces++;
				curMoved--;
			}

			if ( numSpaces >= 4 )
			{
				numSpaces = 0;
				hText.InsertBefore( parserPos, L'\t' );
				curMoved++;
			}

			parserPos--;
		}
		*/
	}

	if ( !bFast )
		OnTextChanged();

	return curMoved;
}
void _smartRow::InsertString( const wchar_t *sz, int pos, int iLength, bool bFast )
{
	if ( IsLocked() )
		return;

	Assert( sz != NULL );

#if DEBUG
	for ( int i = 0; ( (iLength >= 0) ? (sz && i < iLength) : (sz && sz[i]) ); i++ )
		Assert( sz[i] != L'\n' && sz[i] != L'\r' );
#endif

	if ( pos < 0 )
		pos = hText.Count();

	if ( iLength >= 0 )
	{
		if ( pos < hText.Count() )
			hText.InsertMultipleBefore( pos, iLength, sz );
		else
			hText.AddMultipleToTail( iLength, sz );

		if ( !bFast )
			OnTextChanged();

		return;
	}

#if DEBUG
	for ( unsigned int i = 0; i < wcslen( sz ); i++ )
		Assert( sz[i] != L'\n' );
#endif

	for ( unsigned int i = 0; i < wcslen( sz ); i++ )
	{
		InsertChar( sz[i], pos, true );
		pos++; // = hText.Count();
	}

	if ( !bFast )
		OnTextChanged();
}
void _smartRow::InsertString( const char *sz, int pos, int iLength, bool bFast )
{
	int len = Q_strlen( sz ) + 1;
	wchar_t *pWStr = new wchar_t[ len ];
	Q_UTF8ToUnicode( sz, pWStr, len * sizeof(wchar_t) );
	InsertString( pWStr, pos, iLength, bFast );
	delete [] pWStr;
}

void _smartRow::PerformShiftTab( bool bIndent )
{
	if ( IsLocked() )
		return;

	ConvertSpaceToTab();

	if ( bIndent )
		InsertChar( L'\t', 0 );
	else if ( hText.Count() && hText[ 0 ] == L'\t' )
	{
		hText.Remove( 0 );
		OnTextChanged();
	}
}

void _smartRow::ConvertSpaceToTab( int start )
{
	bool bEarlyOut = true;
	int SingleCWidth = GetCharWidth( L' ' );
	//int end = start;

	//	( 4 - ( (rowPos/m_iCached_Charwidth) % 4 ) )

	for ( int i = start; i < hText.Count(); i++ )
	{
		if ( !hText[ i ] )
			continue;

		if ( hText[ i ] == L' ' )
			bEarlyOut = false;
		else if ( !IS_SPACE( hText[ i ] ) )
			break;
	}

	__ROWCALC_INTRO( hText.Count() );
			//if ( hText[i] && !IS_SPACE( hText[i] ) )
			//	break;
			//else if ( hText[i] == L' ' )
			//	bEarlyOut = false;
	__ROWCALC_SPLIT_INTRO();
	__ROWCALC_SPLIT_OUTRO();
			if ( i >= start )
				break;
	__ROWCALC_OUTRO();

	if ( bEarlyOut )
		return;

	int startDist = cur_x;

	for ( int i = start; i < hText.Count(); i++ )
	{
		if ( !hText[i] )
			continue;

		if ( !IS_SPACE( hText[i] ) )
			break;

		int cur_width = SingleCWidth;

		if ( hText[i] == '\t' )
			cur_width *= ( 4 - ( (cur_x/cur_width) % 4 ) );

		cur_x += cur_width;
	}

	cur_x -= startDist;
	int slotsOccupied = cur_x / SingleCWidth;

	for ( int i = start; i < hText.Count(); i++ )
	{
		if ( !hText[ i ] || IS_SPACE( hText[ i ] ) )
		{
			hText.Remove( i );
			i--;
		}
		else
			break;
	}

	int tgTabs = slotsOccupied / 4 + ( (slotsOccupied % 4 != 0) ? 1 : 0 );

	for ( int i = 0; i < tgTabs; i++ )
		InsertChar( L'\t', start );
}
void _smartRow::RemoveLeadingSpaces()
{
	ConvertSpaceToTab();

	while ( hText.Count() )
	{
		if ( hText[0] == L'\t' )
			hText.Remove( 0 );
		else
			break;
	}
}
wchar_t *_smartRow::CopyLeadingSpaces()
{
	int x1 = -1;

	for ( int i = 0; i < hText.Count(); i++ )
	{
		if ( !hText[ i ] )
			continue;

		if ( !IS_SPACE( hText[ i ] ) )
			break;

		x1++;
	}

	if ( x1 < 0 )
		return NULL;

	return ReadInterval( 0, x1, false, true );
}

wchar_t *_smartRow::ReadInterval( int start, int end, bool bAllowLF, bool bIgnoreLF )
{
	if ( hText.Count() < 1 )
	{
		wchar_t *ret = NULL;
		if ( bAllowLF )
		{
			ret = new wchar_t[ 2 ];
			ret[0] = L'\n';
			ret[1] = L'\0';
		}
		return ret;
	}

	if ( start < 0 )
		start = 0;
	if ( end < 0 )
		end = hText.Count();

	//bool bEndLF = bAllowCR;

	//if ( bEndLF && 
	//	( ( start != 0 || end != hText.Count() ) )
	//	)
	//	bEndLF = false;

	if ( bIgnoreLF )
	{
		for ( int i = 0; i < hText.Count() && i < start; i++ )
		{
			if ( !hText[i] )
			{
				start++;
				end++;
			}
		}
	}

	//Assert( start <= end );

	CUtlVector< wchar_t >internalText;

	for ( int i = start; i <= end; i++ )
	{
		if ( i >= hText.Count() )
		{
			if ( bAllowLF )
			{
				internalText.AddToTail( L'\n' );
				break;
			}
		}
		else if ( !hText[i] )
		{
			if ( bIgnoreLF )
				end++;
		}
		else // if ( hText[i] )
		//else
			internalText.AddToTail( hText[i] );
	}

	//Assert( end <= hText.Count() );

	//if ( bEndLF )
	//	internalText.AddToTail( L'\n' );

	internalText.AddToTail( L'\0' );

	Assert( internalText.Count() );

	wchar_t *out = new wchar_t[ internalText.Count() ];
	Q_memset( out, 0, sizeof( wchar_t ) / sizeof( char ) * internalText.Count() );

	for ( int i = 0; i < internalText.Count(); i++ )
	{
		out[ i ] = internalText[ i ];
	}

	internalText.Purge();

	return out;
}
void _smartRow::DeleteInterval( int start, int end, bool bFast )
{
	if ( IsLocked() )
		return;

	if ( end < 0 )
		end = hText.Count() - 1;

	while( start < hText.Count() && start <= end )
	{
		hText.Remove( start );
		end--;
	}

	if ( !bFast )
		OnTextChanged();
}
void _smartRow::DeleteChar( int x, bool bAfter, bool bFast )
{
	if ( IsLocked() )
		return;

	if ( hText.Count() < 1 )
		return;

	x = clamp( x, bAfter ? 0 : 1, hText.Count() - ( bAfter ? 1 : 0 ) );

	if ( !bAfter )
		x--;

	Assert( x >= 0 && x < hText.Count() );

	hText.Remove( x );

	if ( !bFast )
		OnTextChanged();
}
_smartRow *_smartRow::Split( int pos )
{
	if ( IsLocked() )
		return NULL;

	_smartRow *row = new _smartRow( _font, m_iInteriorWidth, m_iInsetWidth );

	if ( pos < hText.Count() )
	{
		wchar_t *text = ReadInterval( pos, -1 );
		row->InsertString( text );
		delete [] text;
	

		DeleteInterval( pos, -1 );
	}

	return row;
}

void _smartRow::OnTextChanged( bool bOnlyLayout )
{
	KillLineBreaks();

	UpdateLinebreaks();

	MakeLayoutDirty();

	if ( !bOnlyLayout )
		MakeTextDirty();
}

bool _smartRow::IsTextDirty()
{
	return m_bDirty;
}
void _smartRow::MakeTextDirty()
{
	m_bDirty = true;
}
void _smartRow::MakeTextClean( const bool bStringLiterals, const bool bBrackets, const bool bHLSL )
{
	if ( !m_bDirty )
		return;

	DoSyntaxHighlighting(bStringLiterals, bBrackets, bHLSL);

	m_bDirty = false;
}

bool _smartRow::IsLayoutDirty_CharWidth()
{
	return m_bDirty_CharWidth;
}
bool _smartRow::IsLayoutDirty_Renderlen()
{
	return m_bDirty_Renderlen;
}
void _smartRow::MakeLayoutDirty()
{
	m_bDirty_CharWidth = true;
	m_bDirty_Renderlen = true;
}
void _smartRow::MakeLayoutClean_CharWidth()
{
	m_bDirty_CharWidth = false;
}
void _smartRow::MakeLayoutClean_Renderlen()
{
	m_bDirty_Renderlen = false;
}

bool _smartRow::IsHighlightRefDirty()
{
	return m_bDirty_HighlightRef;
}
void _smartRow::MakeHighlightRefDirty()
{
	m_bDirty_HighlightRef = true;
}
void _smartRow::MakeHighlightRefClean()
{
	m_bDirty_HighlightRef = false;

	if ( m_pszHighlightRef )
	{
		for (int i = 0; i < hText.Count(); i++)
		{
			const wchar_t *pBase = hText.Base() + i;
			const wchar_t *pRead = m_pszHighlightRef;
			int steps = 0;

			while ( *pRead && *pBase )
			{
				if ( *pRead != *pBase )
				{
					pRead = NULL;
					break;
				}

				pRead++;
				pBase++;
				steps++;
			}

			int maxStep = i + steps;

			if ( pRead != NULL && steps > 0 &&
				( i == 0 || !IS_CHAR(hText[i-1])&&!IS_NUMERICAL(hText[i-1])) &&
				(maxStep >= hText.Count() || !IS_CHAR(hText[maxStep])&&!IS_NUMERICAL(hText[maxStep]) )
				)
			{
				text_cursor cmin( i, 0 );
				text_cursor cmax( i + steps, 0 );

				ToRealCursor( cmin );
				ToRealCursor( cmax );

				_inlineFormat *pFormat = new _inlineFormat();
				pFormat->start = cmin.x;
				pFormat->end = cmax.x;
				pFormat->bOverride_bg = true;
				pFormat->col_bg.SetColor( 80, 65, 40, 255 );
				hUserHighlight.AddToHead( pFormat );
				
				i += steps - 1;
			}
		}
	}
}