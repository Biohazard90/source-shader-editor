#ifndef C_SMART_ROW_H
#define C_SMART_ROW_H

#include "vSheets.h"
#include <vgui/isystem.h>

//#define ST_TEXT_INSET 48

#define DEFAULT_ROW_CAPACITY 0
#define DEFAULT_ROW_GROWSIZE 64

#define ST_COL_SPACING Color( 32, 128, 196, 255 )
#define ST_COL_SELECTION_BG Color( 255, 128, 40, 255 )
#define ST_COL_SELECTION_FG Color( 16, 16, 16, 255 )
#define ST_COL_CMT_FG Color( 64, 64, 64, 255 )
#define ST_COL_HIGHLIGHT_BRACKET Color( 164, 32, 8, 255 )

#define CR_HACK_CHAR 1 // '\r'

struct text_cursor
{
public:
	text_cursor()
	{
	};

	text_cursor( int x0, int y0 )
	{
		x = x0;
		y = y0;
	};

	int x;
	int y;

	inline bool operator> ( const text_cursor &o ) const
		{ return (*this).y > o.y || (*this).y == o.y && (*this).x > o.x; };
	inline bool operator>= ( const text_cursor &o ) const
		{ return *this > o || *this == o; };
	inline bool operator< ( const text_cursor &o ) const
		{ return (*this).y < o.y || (*this).y == o.y && (*this).x < o.x; };
	inline bool operator<= ( const text_cursor &o ) const
		{ return *this < o || *this == o; }
	inline bool operator== ( const text_cursor &o ) const
		{ return (*this).x == o.x && (*this).y == o.y; };
	inline bool operator!= ( const text_cursor &o ) const
		{ return !( (*this) == o ); };
};


struct _inlineFormat
{
public:
	_inlineFormat();

	bool ShouldOverride( int pos );

	// NOT inclusive
	int start;
	int end;

	bool bOverride_fg;
	bool bOverride_bg;
	Color col_fg;
	Color col_bg;

	bool bCRBlock;
};

struct _cmtCtrl
{
public:
	_cmtCtrl();
	_cmtCtrl( int row, int x0, int x1 );
	bool IsCommented( int x, int y );

	int _y;
	// inclusive
	int _x0, _x1;
};


struct _smartRow
{
public:
	_smartRow( HFont font, const int &w, const int &inset );
	~_smartRow();
	_smartRow( const _smartRow &o );

	void SetFont( HFont f );
	int InsertChar( const wchar_t c, int pos, bool bFast = false, bool bSmartTabs = false );
	void InsertString( const wchar_t *sz, int pos = -1, int iLength = -1, bool bFast = false );
	void InsertString( const char *sz, int pos = -1, int iLength = -1, bool bFast = false );

	void PerformShiftTab( bool bIndent = true );
	void ConvertSpaceToTab( int start = 0 );
	void RemoveLeadingSpaces();
	wchar_t *CopyLeadingSpaces();

	void SetInteriorWidth( const int &w, const int &inset );
	int GetInteriorWidth();
	void DrawRow( const int &x, int &y,
				const Color &col_fg );

	bool IsDynamicSpacingEnabled();
	void SetDynamicSpacingEnabled( bool bEnabled );

private:
	int GetRenderNumRows_Internal( int cursor_x, bool bPartial );

	bool m_bDynamicSpacing_Enabled;

public:
	int GetRenderNumRows( int cursor_x = -1 );
	int GetRenderHeight();
	int GetCharWidth( wchar_t c, int rowPos = 0 );
	const int GetAutoCRWidth();
	void RelativeRowCursorToPanel( const int &cursor_x, int &movex, int &movey );

	int GetWordLeft( int pos );
	int GetWordRight( int pos, bool bPreprocTest = false, bool bStringTest = false );
	int GetTextLen( bool bIgnoreLineBreaks = false );
	void ToRealCursor( text_cursor &c );
	void ToVirtualCursor( text_cursor &c );
	bool HasLinebreakAtCursor( const text_cursor &c );

	int PanelToCursorCalcX( int virtualRow, int pixelx );
	
	void OnTextChanged( bool bOnlyLayout = false );
	void KillLineBreaks();
	void UpdateLinebreaks();

	// inclusive
	wchar_t *ReadInterval( int start = -1, int end = -1, bool bAllowLF = false, bool bIgnoreLF = false );
	void DeleteInterval( int start = 0, int end = -1, bool bFast = false );
	void DeleteChar( int x, bool bAfter = false, bool bFast = false );
	_smartRow *Split( int pos );
	CUtlVector< wchar_t > &Read(){ return hText; };

	void ClearHighlightFormats();
	void AddHighlightFormat( _inlineFormat *format );
	void ClearTopFormats();
	void AddTopFormat( _inlineFormat *format );

	void DoSyntaxHighlighting( const bool bStringLiterals, const bool bBrackets, const bool bHLSL );

	bool IsTextDirty();
	void MakeTextDirty();
	void MakeTextClean( const bool bStringLiterals, const bool bBrackets, const bool bHLSL );

	bool IsLayoutDirty_CharWidth();
	bool IsLayoutDirty_Renderlen();
	void MakeLayoutDirty();
	void MakeLayoutClean_CharWidth();
	void MakeLayoutClean_Renderlen();

	bool IsHighlightRefDirty();
	void MakeHighlightRefDirty();
	void MakeHighlightRefClean();
	void SetHighlightRef( wchar_t *pszRef );

	void SetLocked( bool bLocked );
	bool IsLocked();
	void SetDeletable( bool bDeletable );
	bool IsDeletable();

private:

	bool ShouldSplit( int curpixelpos, int localRowStart, int localRowCur );
	bool IsWordConcat( const wchar_t ref, const wchar_t test, bool bLoose, bool bIsValue );

	HFont _font;
	int m_iInteriorWidth;
	int m_iInsetWidth;

	CUtlVector< wchar_t > hText;

	CUtlVector< _inlineFormat* > hSyntaxFormat;
	CUtlVector< _inlineFormat* > hUserHighlight;
	CUtlVector< _inlineFormat* > hTopHighlight;


	int m_iTex_Arrow;
	int m_iTex_CR;

	wchar_t *m_pszHighlightRef;

	int m_iCached_Charwidth;
	int m_iCached_Renderlen;
	bool m_bDirty;
	bool m_bDirty_CharWidth;
	bool m_bDirty_Renderlen;
	bool m_bDirty_HighlightRef;

	bool m_bLocked;
	bool m_bDeletable;
};

#endif