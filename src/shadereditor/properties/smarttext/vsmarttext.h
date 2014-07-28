#ifndef C_SMART_TEXT_H
#define C_SMART_TEXT_H

#include "vSheets.h"
#include <vgui_Controls/scrollbar.h>
#include <vgui/isystem.h>
#include "vSmartRow.h"
#include "vSmartObjectList.h"

#define MAX_SMARTEDIT_HISTORY 100

typedef unsigned int HHISTORYIDX;

class __handleFontCache
{
public:
	__handleFontCache();
	~__handleFontCache();

	void AllocFonts();
	HFont GetFont( int realSize );
	HFont GetTooltipFont();

	const int fSizeDefault() const { return 17; };
	const int fSizeMin() const { return 7; };
	const int fSizeMax() const { return 67; };
	const int fStepSize() const { return 5; };

	int numFonts;
	void InvalidateFonts();

private:
	int CreateMonospaced( HFont &font, const int size );

	HFont m_iTooltipFont;
	bool m_bFontsReady;
};

__handleFontCache *GetFontCacheHandle();

class CSmartAutocomplete;
class CSmartTooltip;

struct __sTextHistory
{
public:
	__sTextHistory( HHISTORYIDX index )
	{
		iIndex = index;
		cursor = text_cursor( 0, 0 );
	};
	~__sTextHistory()
	{
		rows.PurgeAndDeleteElements();
	};
	__sTextHistory( const __sTextHistory &o )
	{
		cursor = o.cursor;

		for ( int i = 0; i < o.rows.Count(); i++ )
			rows.AddToTail( new _smartRow( *o.rows[i] ) );
	};

	CUtlVector< _smartRow* > rows;
	text_cursor cursor;

	HHISTORYIDX iIndex;
};

class CSmartText : public Panel
{
	DECLARE_CLASS_SIMPLE( CSmartText, Panel );

	static HHISTORYIDX sm_iHistoryIndex;
	HHISTORYIDX m_iActiveHistoryIndex;

public:
	enum CodeEditMode_t
	{
		CODEEDITMODE_HLSL = 0,
		CODEEDITMODE_VMT,
	};

	CSmartText( Panel *parent, const char *name, CodeEditMode_t mode );
	~CSmartText();

	void DoFullParserUpdate( int hlslhierachy_flags );
	void SetEnvInfo( int flags );

	void ApplySchemeSettings( IScheme *pScheme );
	void OnSizeChanged( int wide, int tall );
	void OnMoved();
	void OnLayoutUpdated();

	void Paint();
	void OnThink();

	void OnKeyCodeTyped( KeyCode code );
	void OnKeyTyped( wchar_t c );

	void OnMousePressed( MouseCode code );
	void OnMouseDoublePressed( MouseCode code );
	void OnMouseReleased( MouseCode code );
	void OnCursorMoved( int x, int y );
	void OnCursorEntered();
	void OnCursorExited();
	void OnMouseCaptureLost();
	void OnMouseWheeled( int d );

	MESSAGE_FUNC( OnMouseFocusTicked, "OnMouseFocusTicked" );
	MESSAGE_FUNC_PTR( OnMenuItemSelected, "MenuItemSelected", panel );

	_smartRow *InsertRow( const wchar_t *text = NULL, int y = -1 );
	_smartRow *InsertRow( const char *text, int y = -1 );
	void DeleteRow( int y = -1 );
	bool SplitRow( text_cursor pos );
	void CalcSmartTabs( int y, bool bOnTyping = false );
	void SetRowLocked( bool bLocked, int y = -1 );
	void SetRowDeletable( bool bDeletable, int y = -1 );
	bool IsRowLocked( int y = -1 );
	bool IsRowDeletable( int y = -1 );
	int FindRow( _smartRow *p );
	int GetLockedRow( int iNumLocked );
	void ClearDynamicSpacing();

	bool MoveCursor( int leftright, int updown = 0,
					bool bWordJump = false, bool bSelectRange = false, text_cursor *c = NULL );
	void MoveCursorTo( int x, int y );
	void MoveCursorTo( text_cursor c );
	void MoveCursorToEndOfRow( int y = -1, text_cursor *c = NULL );
	void MoveCursorToEndOfText( text_cursor *c = NULL );

	static HFont *m_pFontRef;

	//MESSAGE_FUNC( OnKillFocus, "KillFocus" );

	void SetLiveParseEnabled( bool bEnabled );
	void SetACEnabled( bool bEnabled );
	void SetSyntaxHighlightEnabled( bool bEnabled );

	bool IsLiveParseEnabled();
	bool IsACEnabled();
	bool IsSyntaxHighlightEnabled();

	const bool IsHLSL();
	const bool IsVMT();

private:

	void InitColors();
	Color _col_SelectionFG;
	Color _col_SelectionBG;

	bool m_bEnableLiveParse;
	bool m_bEnableAC;
	bool m_bEnableSyntaxHighlight;

	CodeEditMode_t mode;

	void OnRowEdit( int y = -1 );
	void OnRowEnter( int y = -1, bool bNoTabs = false );

	void UpdateScrollbar( int lastRealRow = -1 );
	void OnTextChanged( int lastRealRow = -1 );
	void ValidateCursor( text_cursor *c = NULL );

	int CalcInteriorWidth();
	void CalcCurrentRenderRow( int &realRow, int &virtualRow );

	int FontTall();

	long m_iBlinkTime;
	void ResetBlinkTime();

	void UpdateFont();
	HFont m_Font;
	int m_iFontSize;
	int m_iTextInset;
	int m_iEnvInfo;
	void ChangeFontSize( int delta );

	text_cursor cur;
	text_cursor select[2];
	bool m_bSelectionValid;

	void ProcessDelete( bool bAfter = false );
	void ProcessCut();
	void ProcessCopy();
	void ProcessPaste();


	void UpdateHighlightFormats();
	void UpdateSelectionFormat();
	void OnSelectionChanged();

	void Selection_Reset();
	void Selection_Select( text_cursor pos );
	void Selection_AddRow( int y );
	void Selection_Delete();
public:
	void ParseInsertText( const wchar_t *str );
	void ParseInsertText( const char *str );
	void Selection_Copy( text_cursor *cMin = NULL, text_cursor *cMax = NULL, wchar_t **pBuffOut = NULL, bool bAddCR = false ); // pBuffOut takes ownership
private:
	void Selection_GetSorted( text_cursor *v );
	bool Selection_Active(){ return m_bSelectionValid; };
	bool Selection_Valid(){ return Selection_Active() && select[0] != select[1]; };
	bool Selection_GetRowRange( const int &row, int &x_min, int &x_max,
								text_cursor *cMin = NULL, text_cursor *cMax = NULL );
	wchar_t *m_pszLastSelectedWord;

	int m_iXCursorHistory;
	void StompCursorHistory();
	
	bool m_bIsSelecting;
	bool m_bIsRowSelecting;
	long scrollDelay;
	ScrollBar *m_pScrollBar_V;
	CUtlVector< _smartRow* > hRows;

	void CursorToPanel( const text_cursor &c, int &x, int &y );
	void PanelToCursor( const int &x, const int &y, text_cursor &c );

	void AdvanceScrollbarOnDemand();

	void KillCmtList();
	void GenerateCmtList();
	void UpdateCmtFormat();
	bool IsCommented( int x, int y );
	CUtlVector< _cmtCtrl > m_hCmtList;

	bool m_bTopFormatsDirty;
	bool m_bTopHighlightLock;
	bool m_bTopHighlightOnTyped;

	void UpdateTopFormats( bool bDoHighlights = true );
	void MakeTopFormatsDirty();
	bool IsTopFormatsDirty();



	// ac control
	enum
	{
		ACTOOLTIP_NONE = 0,
		ACTOOLTIP_FUNC,
		ACTOOLTIP_OBJECT,
	};

	CSmartObjectList *m_pGlobalDatabase;

	void CreateACMenu( int iTooltipMode = ACTOOLTIP_NONE );
	void DestroyACMenu( bool bMenu = true, bool bTooltip = true );
	void DestroyHoverTooltip();
	void CreateHoverTooltipThink();

	void OnACMenuCreationFailed();
	bool OnACCatchKeyCode( KeyCode code );
	bool OnACKeyTyped( wchar_t c );

	void AddEnvInfoToDB();

	void InsertFromAC( const char *pIdent = NULL );
	void CalcACPos( const text_cursor &c, int &x, int &y, int menuSizeX, int menuSizeY );
	void ReleaseTooltipOnDemand();
	bool ShouldReleaseTooltip();
	bool GetWordLeft( text_cursor &c );

	text_cursor m_curACStart;
	text_cursor m_curHoverMin, m_curHoverMax;
	CSmartAutocomplete *m_pACMenu;
	CSmartTooltip *m_pFuncTooltip;
	CSmartTooltip *m_pHoverTooltip;
	bool m_bAllowHoverTooltip;
	double m_lLastCursorMoved;

	void UpdateTooltipPositions();

	// live ac
	bool m_bCodeDirty;
	float m_flLastCodeParse;

	void MakeCodeDirty();
	bool IsCodeDirty();
	void UpdateParseCode();



	int m_iTex_Lock;

	// history
	bool IsHistoryDirty();
	void MakeHistoryDirty();
	void OnHistoryDirty();

	void FlushHistory( bool bOld = true, bool bNew = true );
	void PushHistory( int dir = 1 );
	void PopHistory( int dir = 1 );

	__sTextHistory *CreateHistoryEntry();
	void ApplyHistoryEntry( __sTextHistory *history );

	CUtlVector< __sTextHistory* >hHistory_Old;
	CUtlVector< __sTextHistory* >hHistory_New;
	bool m_bHistoryDirty;

public:
	HHISTORYIDX GetCurrentHistoryIndex();

private:



	bool CanRead( text_cursor *c = NULL );
	const wchar_t &Read( text_cursor *c = NULL );
	void GetScopeData( text_cursor c, int &iScopeDepth, int &iScopeNum, int &iTextPos );
};


#endif