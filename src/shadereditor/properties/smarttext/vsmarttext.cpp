
#include "cbase.h"
#include "vSmartText.h"
#include "vSmartObject.h"
#include "vSmartObjectList.h"
#include "vSmartAutocomplete.h"
#include "vSmartTooltip.h"
#include "vgui_controls/menuitem.h"
#include "cRegex.h"
#include <ctype.h>

//#define UCHAR_MDOT L'\x00B7'
//#define UCHAR_ARROW_R L'\x2192'
//#define UCHAR_AUTO_CR L'\x2514'

HFont *CSmartText::m_pFontRef = NULL;

enum
{
	PARSERREQUEST_INVALID = 0,

	PARSERREQUEST_PS = ( 1 << 0 ),
	PARSERREQUEST_VS = ( 1 << 1 ),
	PARSERREQUEST_USER = ( 1 << 2 ),

	PARSERREQUEST_STOPTHREAD = ( 1 << 31 ),
	PARSERREQUEST_,
};

struct ParserThreadCmd_Request
{
	ParserThreadCmd_Request(){
		pszMem = NULL;
		pParentDatabase = NULL;
	};

	//CSmartText *pTarget;
	int flags;
	wchar_t *pszMem;
	CSmartObjectList *pParentDatabase;
};
struct ParserThreadCmd_Response
{
	ParserThreadCmd_Response(){};
	//CSmartText *pTarget;
	CSmartObjectList *pList;
	int flags;
};

class CParserThread : public CThread
{
public:
	int Run();

	CMessageQueue< ParserThreadCmd_Request* > m_QueueParse;

	// take ownership of mem
	static void RequestUpdate( /*CSmartText *target,*/ int flags = PARSERREQUEST_PS | PARSERREQUEST_VS, wchar_t *pszStream = NULL, CSmartObjectList *pParentDB = NULL );
};

class CParserHelper : public CAutoGameSystemPerFrame
{
public:
	CParserHelper();
	~CParserHelper(){
		delete pLastList;
		delete pLiveList;
	};

	CMessageQueue< ParserThreadCmd_Response* > m_QueueResponse;

	void Update( float frametime );
	void Shutdown();

	bool HasDatabaseWaiting(){ return pLastList != NULL; };
	bool HasLiveDatabaseWaiting(){ return pLiveList != NULL; };
	CSmartObjectList *ObtainDatabase(){
											Assert( pLastList != NULL );
											CSmartObjectList *tmp = pLastList;
											pLastList = NULL;
											return tmp;
	};
	CSmartObjectList *ObtainLiveDatabase(){
											Assert( pLiveList != NULL );
											CSmartObjectList *tmp = pLiveList;
											pLiveList = NULL;
											return tmp;
	};

private:
	CSmartObjectList *pLastList;
	CSmartObjectList *pLiveList;
};

static CParserThread ParserThread;
static CParserHelper ParserHelper;

int CParserThread::Run()
{
	for ( bool bFinished = false; !bFinished ;)
	{
		while ( m_QueueParse.MessageWaiting() )
		{
			ParserThreadCmd_Request *pMsg;
			m_QueueParse.WaitMessage( &pMsg );

			bool bExit = pMsg->flags == PARSERREQUEST_STOPTHREAD;

			if ( bExit )
			{
				bFinished = true;

				Assert( !pMsg->pszMem );
				Assert( !pMsg->pParentDatabase );

				delete pMsg;
				break;
			}
#if ENABLE_AUTOCOMP_PARSER
			else
			{
				//while (true)
				//{
				char *pStream = NULL;
				int len = 0;

				if ( pMsg->pszMem )
				{
					len = Q_wcslen( pMsg->pszMem ) + 1;
					if ( len > 1 )
						pStream = new char[ len ];
				}

					CSmartObjectList *l = new CSmartObjectList();

					CUtlVector< char* > hIncludes;

					if ( pStream != NULL )
					{
						CUtlBuffer buf( pStream, len );
						Q_UnicodeToUTF8( pMsg->pszMem, pStream, len );

						CSmartObjectList *pRef = pMsg->pParentDatabase ? pMsg->pParentDatabase : l;

						if ( pRef != l )
							pRef->SetDatabaseCopy( l );

						pRef->ParseCode_Stream( buf, true, NULL, IDENT_LIVE_CODE, Q_strlen(pStream) - 1 );

						buf.Clear();
						delete [] pStream;
					}
					else if ( pMsg->flags & PARSERREQUEST_PS || pMsg->flags & PARSERREQUEST_VS )
					{
						l->ParseCode( "intrinsic.h", true, &hIncludes );
						l->BuildIntrinsicDatatypes();

						if ( pMsg->flags & PARSERREQUEST_VS )
						{
							l->ParseCode( "common_vs_fxc.h", true, &hIncludes );
						}
						if ( pMsg->flags & PARSERREQUEST_PS )
						{
							l->ParseCode( "common_ps_fxc.h", true, &hIncludes );
							l->ParseCode( "common_vertexlitgeneric_dx9.h", true, &hIncludes );
							l->ParseCode( "common_lightmappedgeneric_fxc.h", true, &hIncludes );
							l->ParseCode( "common_flashlight_fxc.h", true, &hIncludes );
							l->ParseCode( "common_parallax.h", true, &hIncludes );
						}
					}
					else
						Assert( 0 );

					for ( int i = 0; i < hIncludes.Count(); i++ )
						delete [] hIncludes[ i ];

					delete [] pMsg->pszMem;
					delete pMsg->pParentDatabase;

					hIncludes.Purge();
				//}

					ParserThreadCmd_Response *pResponse = new ParserThreadCmd_Response();
					//pResponse->pTarget = pMsg->pTarget;
					pResponse->pList = l;
					pResponse->flags = pMsg->flags;
					//pResponse->pList = NULL;
					ParserHelper.m_QueueResponse.QueueMessage( pResponse );
				//}
			}
#endif

			delete pMsg;
		}

		//Yield();
		Sleep( 1 );
	}

	return 0;
}

void CParserThread::RequestUpdate( /*CSmartText *target,*/ int flags, wchar_t *pszStream, CSmartObjectList *pParentDB )
{
	ParserThreadCmd_Request *pR = new ParserThreadCmd_Request();
	pR->flags = flags;
	pR->pszMem = pszStream;
	pR->pParentDatabase = pParentDB;
	//pR->pTarget = target;

	ParserThread.m_QueueParse.QueueMessage( pR );

	if ( !ParserThread.IsAlive() )
		ParserThread.Start();

	//
}


CParserHelper::CParserHelper() : CAutoGameSystemPerFrame( "auto_parser_helper" )
{
	pLastList = NULL;
	pLiveList = NULL;
}

void CParserHelper::Update( float frametime )
{
	while ( m_QueueResponse.MessageWaiting() )
	{
		ParserThreadCmd_Response *pMsg;
		m_QueueResponse.WaitMessage( &pMsg );

		if ( pMsg->flags & PARSERREQUEST_USER )
		{
			delete pLiveList;
			pLiveList = pMsg->pList;
		}
		else
		{
			delete pLastList;
			pLastList = pMsg->pList;
		}

		delete pMsg;
	}
}

void CParserHelper::Shutdown()
{
	if ( ParserThread.IsAlive() )
	{
		ParserThreadCmd_Request *pR = new ParserThreadCmd_Request();
		pR->flags = PARSERREQUEST_STOPTHREAD;
		ParserThread.m_QueueParse.QueueMessage( pR );

		VCRHook_WaitForSingleObject( ParserThread.GetThreadHandle(), TT_INFINITE );
	}
}

static __handleFontCache __hfcInstance;
__handleFontCache *GetFontCacheHandle()
{
	return &__hfcInstance;
}

__handleFontCache::__handleFontCache()
{
	CSmartText::m_pFontRef = NULL;
	m_iTooltipFont = 0;
	m_bFontsReady = false;
}
__handleFontCache::~__handleFontCache()
{
	delete [] CSmartText::m_pFontRef;
}
void __handleFontCache::AllocFonts()
{
	if ( CSmartText::m_pFontRef != NULL && m_bFontsReady )
		return;

	numFonts = ( fSizeMax() - fSizeMin() ) / fStepSize() + 1;

	if ( CSmartText::m_pFontRef == NULL )
	{
		CSmartText::m_pFontRef = new HFont[ numFonts ];
		Q_memset( CSmartText::m_pFontRef, 0, sizeof( unsigned long ) * numFonts );
	}

	int iError = 0;

	for ( int i = 0; i < numFonts; i++ )
		iError = max( iError, CreateMonospaced( CSmartText::m_pFontRef[ i ], fSizeMin() + fStepSize() * i ) );

	iError = max( iError, CreateMonospaced( m_iTooltipFont, 14 ) );

	if ( iError == 2 )
		Warning( "Unable to locate fonts: 'Consolas' (not shipped), 'monofur' (shipped)!\nFont rendering will be compromised.\n" );
	else if ( iError )
		Warning( "Unable to locate font: 'Consolas' (not shipped)!\nFalling back to 'monofur'.\n" );

	m_bFontsReady = true;
}
void __handleFontCache::InvalidateFonts()
{
	m_bFontsReady = false;
}
int __handleFontCache::CreateMonospaced( HFont &font, const int size )
{
	if ( font == 0 )
		font = surface()->CreateFont();

	int flags =
#ifndef SHADER_EDITOR_DLL_SWARM
		ISurface::FONTFLAG_CUSTOM | ISurface::FONTFLAG_ANTIALIAS
#else
		FONTFLAG_CUSTOM | FONTFLAG_ANTIALIAS
#endif
		;

	if ( !surface()->SetFontGlyphSet( font, "Consolas", size, 0, 0, 0, flags ) )
	{
		if ( !surface()->SetFontGlyphSet( font, "monofur", size, 0, 0, 0, flags ) )
			return 2;
		else return 1;
	}
	return 0;
}
HFont __handleFontCache::GetFont( int realSize )
{
	AllocFonts();

	Assert( realSize >= fSizeMin() && realSize <= fSizeMax() );

	realSize -= fSizeMin();
	realSize /= fStepSize();

	Assert( realSize >= 0 && realSize < numFonts );

	return CSmartText::m_pFontRef[ realSize ];
}
HFont __handleFontCache::GetTooltipFont()
{
	AllocFonts();

	return m_iTooltipFont;
}

HHISTORYIDX CSmartText::sm_iHistoryIndex = 0;

CSmartText::CSmartText( Panel *parent, const char *name, CodeEditMode_t mode ) : BaseClass( parent, name )
{
	this->mode = mode;

	InitColors();
	__hfcInstance.AllocFonts();

	//if ( m_pFontRef == NULL )
	//	m_pFontRef = new HFont( surface()->CreateFont() );
	m_bEnableLiveParse = true;
	m_bEnableAC = true;
	m_bEnableSyntaxHighlight = true;

	//m_Font = *m_pFontRef;
	hRows.SetGrowSize( 64 );

	m_iFontSize = ::__hfcInstance.fSizeDefault();
	m_iTextInset = 0;
	m_iEnvInfo = 0;
	m_iActiveHistoryIndex = 0;
	m_bTopHighlightLock = false;
	m_bTopHighlightOnTyped = false;
	m_bTopFormatsDirty = false;

	m_pGlobalDatabase = NULL;
	m_pACMenu = NULL;
	m_pFuncTooltip = NULL;
	m_pHoverTooltip = NULL;
	m_bAllowHoverTooltip = false;

	m_pszLastSelectedWord = NULL;

	m_lLastCursorMoved = 0.0;

	m_bCodeDirty = false;
	m_bHistoryDirty = false;
	m_flLastCodeParse = 0;

	UpdateFont();

	cur.x = cur.y = 0;

	m_iXCursorHistory = 0;
	scrollDelay = 0;
	m_bIsSelecting = false;
	m_bIsRowSelecting = false;
	m_iBlinkTime = 0;

	//input()->OnChangeIMEByHandle( input()->GetEnglishIMEHandle() );
	SetupVguiTex( m_iTex_Lock, "shadereditor/lock" );

	m_pScrollBar_V = new ScrollBar( this, "", true );
	m_pScrollBar_V->AddActionSignalTarget( this );
	m_pScrollBar_V->SetValue(0);

	InsertRow();

	OnLayoutUpdated();
	Selection_Reset();

	SetDropEnabled( true );

	//ParserThread.RequestUpdate();

	MakeHistoryDirty();
}

CSmartText::~CSmartText()
{
	FlushHistory();
	delete m_pGlobalDatabase;
	KillCmtList();
	hRows.PurgeAndDeleteElements();
	DestroyACMenu();
	delete [] m_pszLastSelectedWord;
}

void CSmartText::SetLiveParseEnabled( bool bEnabled )
{
	m_bEnableLiveParse = bEnabled;
}
void CSmartText::SetACEnabled( bool bEnabled )
{
	m_bEnableAC = bEnabled;
}
void CSmartText::SetSyntaxHighlightEnabled( bool bEnabled )
{
	m_bEnableSyntaxHighlight = bEnabled;
}

bool CSmartText::IsLiveParseEnabled()
{
	return m_bEnableLiveParse;
}
bool CSmartText::IsACEnabled()
{
	return m_bEnableAC;
}
bool CSmartText::IsSyntaxHighlightEnabled()
{
	return m_bEnableSyntaxHighlight;
}

const bool CSmartText::IsHLSL()
{
	return mode == CODEEDITMODE_HLSL;
}
const bool CSmartText::IsVMT()
{
	return mode == CODEEDITMODE_VMT;
}

void CSmartText::DoFullParserUpdate( int hlslhierachy_flags )
{
	int flags = 0;

	if ( hlslhierachy_flags & HLSLHIERACHY_PS )
		flags |= PARSERREQUEST_PS;

	if ( hlslhierachy_flags & HLSLHIERACHY_VS )
		flags |= PARSERREQUEST_VS;

	if ( !flags )
		flags |= PARSERREQUEST_PS | PARSERREQUEST_VS;

	ParserThread.RequestUpdate( flags );
}

void CSmartText::SetEnvInfo( int flags )
{
	m_iEnvInfo = flags;
}

void CSmartText::UpdateFont()
{
	//m_Font = surface()->CreateFont();
	m_Font = __hfcInstance.GetFont( m_iFontSize );
}

void CSmartText::ChangeFontSize( int delta )
{
	m_iFontSize = clamp( m_iFontSize + delta * __hfcInstance.fStepSize(),
			__hfcInstance.fSizeMin(),
			__hfcInstance.fSizeMax() );
	UpdateFont();
	OnLayoutUpdated();
}
void CSmartText::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundEnabled( true );
	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( true );

	InitColors();
	_col_SelectionBG = pScheme->GetColor( "SmartText.SelectionBG", _col_SelectionBG );
	_col_SelectionFG = pScheme->GetColor( "SmartText.SelectionFG", _col_SelectionFG );
	
	SetBorder( pScheme->GetBorder("ButtonDepressedBorder") );

	UpdateScrollbar();
}

void CSmartText::InitColors()
{
	_col_SelectionBG = ST_COL_SELECTION_BG;
	_col_SelectionFG = ST_COL_SELECTION_FG;
}

void CSmartText::OnSizeChanged( int wide, int tall )
{
	DestroyACMenu();

	//BaseClass::OnSizeChanged( wide, tall );
	InvalidateLayout(true);

	OnLayoutUpdated();
}
void CSmartText::OnMoved()
{
	DestroyACMenu();
}
void CSmartText::OnLayoutUpdated()
{
	int lastRealRow, lastVRow;
	CalcCurrentRenderRow( lastRealRow, lastVRow );

	m_iTextInset = surface()->GetCharacterWidth( m_Font, ' ' ) * 6;

	int rowwide = CalcInteriorWidth();
	int iOffset = ( hRows.Count() && hRows[ 0 ]->GetInteriorWidth() > rowwide ) ? 0 : 1;

	int oldVRow = hRows[ cur.y ]->GetRenderNumRows( cur.x - iOffset );
	for ( int i = 0; i < hRows.Count(); i++ )
	{
		hRows[ i ]->SetFont( m_Font );
		hRows[i]->SetInteriorWidth( rowwide, m_iTextInset );
	}
	int newVRow = hRows[ cur.y ]->GetRenderNumRows( cur.x - iOffset );

	cur.x += newVRow - oldVRow;

	OnTextChanged( lastRealRow );
}

int CSmartText::CalcInteriorWidth()
{
	int sx,sy,scrollbar_sx,scrollbar_sy;
	GetSize( sx, sy );
	m_pScrollBar_V->GetSize( scrollbar_sx, scrollbar_sy );
	return sx - scrollbar_sx - m_iTextInset;
}

void CSmartText::UpdateScrollbar( int lastRealRow )
{
	int sx, sy;
	GetSize( sx, sy );

	m_pScrollBar_V->SetSize( 20, sy );
	m_pScrollBar_V->SetPos( sx - 20, 0 );

	//int iLastValue = m_pScrollBar_V->GetValue();
	int iNewValue = m_pScrollBar_V->GetValue();
	const bool bPerformLayoutTest = lastRealRow >= 0;

	int rows = 0;
	for ( int i = 0; i < hRows.Count(); i++ )
	{
		if ( bPerformLayoutTest && lastRealRow == i )
			iNewValue = rows;

		rows += hRows[i]->GetRenderNumRows();
	}

	//if ( m_pScrollBar_V->GetValue() > rows )
	//	m_pScrollBar_V->SetValue( rows );

	int iBloat = max( 6, (int)( rows * 0.1f ) );
	m_pScrollBar_V->SetRange( 0, rows + iBloat );
	m_pScrollBar_V->SetRangeWindow( 1 + iBloat );
	m_pScrollBar_V->InvalidateLayout(true);

	if ( bPerformLayoutTest )
		m_pScrollBar_V->SetValue( iNewValue );
}

void CSmartText::Paint()
{
	BaseClass::Paint();

	int scrollbar_sx, scrollbar_sy;
	m_pScrollBar_V->GetSize( scrollbar_sx, scrollbar_sy );

	int sx, sy;
	GetSize( sx, sy );

	int virtualRow, realRow;
	CalcCurrentRenderRow( realRow, virtualRow );

	const int ifonttall = FontTall();
	int xpos = m_iTextInset; //sx - scrollbar_sx - rowwide;
	int ypos = virtualRow * -ifonttall;
	int cWidth = surface()->GetCharacterWidth( m_Font, ' ' );
	const int iLockInset = cWidth * 0.5f;

	Color col_fg( 230, 230, 232, 255 );

	int linex = xpos - (int)( m_iTextInset * 0.05f );

	surface()->DrawSetColor( Color( 24, 24, 24, 255 ) );
	surface()->DrawFilledRect( 0, 0, linex, sy );

	surface()->DrawSetColor( col_fg );
	surface()->DrawFilledRect( linex, 0, linex + 1, sy );

	surface()->DrawSetTextFont( m_Font );

	for ( int i = realRow; i < hRows.Count(); i++ )
	{
		int rowNum = i + 1;
		int numPos = 1 + cWidth * 5;

		/*
		for ( int insCalc = 1000; insCalc > 0; insCalc /= 10 )
			if ( rowNum / insCalc > 0 )
				break;
			else
				numPos += cWidth;
				*/
		for ( int insCalc = 1; ( rowNum / insCalc ) > 0; insCalc *= 10 )
			numPos -= cWidth;

		if ( i <= 99999 )
		{
			char numStrC[6];
			wchar_t numStrLC[6];
			Q_snprintf( numStrC, sizeof( numStrC ), "%i", rowNum );
			Q_UTF8ToUnicode( numStrC, numStrLC, sizeof( numStrLC ) );

			surface()->DrawSetTextPos( numPos, ypos );
			surface()->DrawSetTextColor( ST_COL_SPACING );
			surface()->DrawPrintText( numStrLC, Q_strlen( numStrC ) );
		}

		if ( hRows[ i ]->IsTextDirty() ) //&& IsSyntaxHighlightEnabled() )
			hRows[ i ]->MakeTextClean( true, true, IsSyntaxHighlightEnabled() );


		if ( hRows[ i ]->IsLocked() )
		{
			surface()->DrawSetColor( Color( 60, 65, 68, 255 ) );
			surface()->DrawFilledRect( xpos, ypos, sx - scrollbar_sx,
				ypos + hRows[ i ]->GetRenderHeight() );

			surface()->DrawSetTexture( m_iTex_Lock );
			surface()->DrawSetColor( Color( 220, 200, 10, 255 ) );

			surface()->DrawTexturedRect( 0, ypos + iLockInset,
				ifonttall - iLockInset * 2, ypos + ifonttall - iLockInset );
		}

		hRows[ i ]->DrawRow( xpos, ypos, col_fg );

		if ( ypos > sy )
			break;
	}

	long blinkDelta = system()->GetTimeMillis() - m_iBlinkTime;

	if ( (blinkDelta / 300) % 2 == 0 )
	{
		surface()->DrawSetColor( col_fg );

		int cx, cy;
		CursorToPanel( cur, cx, cy );
		int fontHeight = FontTall();

		surface()->DrawFilledRect( cx, cy, cx + 1, cy + fontHeight );
	}

	/*int mx, my;
	input()->GetCursorPosition( mx, my );
	ScreenToLocal( mx, my );
	text_cursor cc;
	PanelToCursor( mx, my, cc );
	int cx, cy;
	CursorToPanel( cc, cx, cy );

	surface()->DrawSetColor( Color( 255, 0, 0, 255 ) );
	int fontHeight = FontTall();
	surface()->DrawFilledRect( cx, cy, cx + 1, cy + fontHeight );*/
}

void CSmartText::CalcCurrentRenderRow( int &realRow, int &virtualRow )
{
	virtualRow = m_pScrollBar_V->GetValue();
	int tmpVRow = virtualRow;
	realRow = 0;

	for (;;)
	{
		Assert( realRow >= 0 && realRow < hRows.Count() );
		if ( realRow >= hRows.Count() )
			break;

		tmpVRow -= hRows[ realRow ]->GetRenderNumRows();

		if ( tmpVRow < 0 )
			break;

		virtualRow = abs( tmpVRow );
		realRow++;
	}
}

void CSmartText::ResetBlinkTime()
{
	m_iBlinkTime = system()->GetTimeMillis();
}

void CSmartText::OnKeyCodeTyped( KeyCode code )
{
	ResetBlinkTime();

	DestroyHoverTooltip();

	if ( OnACCatchKeyCode( code ) )
		return;

	const bool shift = ( input()->IsKeyDown( KEY_LSHIFT ) || input()->IsKeyDown( KEY_RSHIFT ) );
	const bool ctrl = ( input()->IsKeyDown( KEY_LCONTROL ) || input()->IsKeyDown( KEY_RCONTROL ) );

	bool bMoving = false;
	bool bCtn_Shift = code == KEY_LSHIFT || code == KEY_RSHIFT;
	bool bCtn_Ctrl = code == KEY_LCONTROL || code == KEY_RCONTROL;

	switch ( code )
	{
	case KEY_ENTER:
		{
			bool bSomethingChanged = Selection_Active() && select[0] != select[1];

			Selection_Delete();

			if ( SplitRow( cur ) )
			{
				cur.x = 0;
				cur.y++;
				ValidateCursor();

				if ( !hRows[cur.y]->GetTextLen() )
					hRows[cur.y]->SetDynamicSpacingEnabled( true );

				CalcSmartTabs( cur.y );
				AdvanceScrollbarOnDemand();

				MakeTopFormatsDirty();
				ReleaseTooltipOnDemand();

				bSomethingChanged = true;
			}

			if ( bSomethingChanged )
				MakeHistoryDirty();
		}
		break;
	case KEY_LEFT:
		{
			int lastRow = cur.y;
			MoveCursor( -1, 0, ctrl, shift );
			AdvanceScrollbarOnDemand();
			bMoving = true;
			ReleaseTooltipOnDemand();

			if ( lastRow != cur.y )
				OnRowEnter(-1,true);
		}
		break;
	case KEY_RIGHT:
		{
			int lastRow = cur.y;
			MoveCursor( 1, 0, ctrl, shift );
			AdvanceScrollbarOnDemand();
			bMoving = true;
			ReleaseTooltipOnDemand();

			if ( lastRow != cur.y )
				OnRowEnter(-1,true);
		}
		break;
	case KEY_UP:
			MoveCursor( 0, -1, ctrl, shift );
			AdvanceScrollbarOnDemand();
			bMoving = true;
			ReleaseTooltipOnDemand();
			OnRowEnter();
		break;
	case KEY_DOWN:
			MoveCursor( 0, 1, ctrl, shift );
			AdvanceScrollbarOnDemand();
			bMoving = true;
			ReleaseTooltipOnDemand();
			OnRowEnter();
		break;
	case KEY_HOME:
		{
			if ( ctrl )
			{
				cur.y = 0;
				cur.x = 0;
			}
			else
			{
				text_cursor firstChar( 0, cur.y );
				while ( CanRead(&firstChar) && IS_SPACE( Read(&firstChar) ) )
					MoveCursor( 1, 0, false, false, &firstChar );

				if ( (cur.x == 0 || firstChar.x < cur.x) &&
					firstChar.x != 0 &&
					firstChar.x != hRows[firstChar.y]->GetTextLen() - 1 )
					cur.x = firstChar.x;
				else
					cur.x = 0;
			}

			ValidateCursor();
			bMoving = true;
		}
		break;
	case KEY_END:
			if ( ctrl )
				cur.y = hRows.Count() - 1;

			MoveCursorToEndOfRow();
			ValidateCursor();
			bMoving = true;
		break;
	case KEY_LSHIFT:
	case KEY_RSHIFT:
			bMoving = true;
		break;
	case KEY_LCONTROL:
	case KEY_RCONTROL:
			bMoving = true;
		break;
	case KEY_DELETE:
	case KEY_BACKSPACE:
			ProcessDelete( code == KEY_DELETE );
			OnACKeyTyped( 0 );
		break;
	case KEY_X:
			if ( ctrl )
				ProcessCut();
		break;
	case KEY_C:
			if ( ctrl )
				ProcessCopy();
		break;
	case KEY_V:
			if ( ctrl )
				ProcessPaste();
		break;
	case KEY_Z:
			if ( ctrl )
				PopHistory( shift ? -1 : 1 );
		break;
	case KEY_Y:
			if ( ctrl )
				PopHistory(-1);
		break;
	case KEY_A:
			if ( ctrl )
			{
				MoveCursorToEndOfRow( hRows.Count() - 1 );
				Selection_Reset();
				Selection_Select( text_cursor( 0, 0 ) );
				Selection_Select( cur );
			}
		break;
	case KEY_0:
	case KEY_PAD_0:
			if ( ctrl )
			{
				m_iFontSize = ::__hfcInstance.fSizeDefault();
				ChangeFontSize( 0 );
			}
		break;
	case KEY_PAD_PLUS:
			if ( ctrl )
				ChangeFontSize( 1 );
		break;
	case KEY_MINUS:
	case KEY_PAD_MINUS:
			if ( ctrl )
				ChangeFontSize( -1 );
		break;
	}

	if ( bMoving )
		MakeTopFormatsDirty();

	if ( bMoving && ( !bCtn_Shift || !Selection_Active() ) )
	{
		if ( shift )
			Selection_Select( cur );
		else if ( !bCtn_Ctrl )
			Selection_Reset();
	}

	//BaseClass::OnKeyCodeTyped( code );
}
void CSmartText::OnKeyTyped( wchar_t c )
{
	const bool bTab = c == 9;
	const bool shift = ( input()->IsKeyDown( KEY_LSHIFT ) || input()->IsKeyDown( KEY_RSHIFT ) );
	const bool ctrl = ( input()->IsKeyDown( KEY_LCONTROL ) || input()->IsKeyDown( KEY_RCONTROL ) );

	if ( iswcntrl( c ) && !bTab ||
		c == 0 )
	{
		if ( !shift && !ctrl )
			Selection_Reset();
		return;
	}

	//if ( OnACKeyTyped( c ) )
	//	return;

	if ( ctrl && c == L' ' )
	{
		CreateACMenu();
		return;
	}

	text_cursor sr[2];
	Selection_GetSorted( sr );

	OnRowEdit();

	if ( bTab && Selection_Valid() &&
				( sr[0].y != sr[1].y ||
				sr[0].x == 0 && sr[1].x == hRows[sr[1].y ]->GetTextLen( true ) ) )
	{
		text_cursor rCur = cur;
		hRows[rCur.y]->ToRealCursor( rCur );

		bool cursorAtEnd = rCur == sr[1];

		if ( sr[1].y > 0 && sr[1].x == 0 )
		{
			sr[1].y--;
			sr[1].x = hRows[ sr[1].y ]->GetTextLen( true );
		}

		for ( int i = sr[0].y; i <= sr[1].y; i++ )
		{
			hRows[ i ]->PerformShiftTab( !shift );
			MakeHistoryDirty();
		}

		Selection_Reset();

		for ( int i = sr[0].y; i <= sr[1].y; i++ )
			Selection_AddRow( i );

		Selection_GetSorted( select );

		if ( cursorAtEnd )
			MoveCursorToEndOfRow( sr[1].y );
		else
			swap( select[0], select[1] );
	}
	else
	{
		ValidateCursor();

		if ( Selection_Valid() )
		{
			cur = sr[0];
			hRows[cur.y]->ToVirtualCursor( cur );
		}

		Selection_Delete();

		text_cursor pos = cur;
		pos.y = clamp( pos.y, 0, hRows.Count() - 1 );

		Assert( pos.y >= 0 && pos.y < hRows.Count() );

		hRows[pos.y]->ToRealCursor( cur );
		hRows[pos.y]->KillLineBreaks();

		int oldLen = hRows[pos.y]->GetTextLen( true );
		hRows[pos.y]->InsertChar( c, cur.x, false, true );
		int newLen = hRows[pos.y]->GetTextLen( true );

		cur.x += newLen - oldLen;

		//hRows[pos.y]->UpdateLinebreaks();
		hRows[pos.y]->ToVirtualCursor( cur );

		ValidateCursor();

		MakeHistoryDirty();
	}

	if ( c == L'#' && cur.x > 1 )
	{
		int iPosFirst = 0;
		for ( ; iPosFirst < hRows[ cur.y ]->Read().Count(); iPosFirst++ )
		{
			if ( !IS_SPACE( hRows[ cur.y ]->Read()[iPosFirst] ) )
				break;
		}

		if ( iPosFirst == cur.x - 1 )
		{
			hRows[ cur.y ]->DeleteInterval( 0, cur.x - 2 );
			cur.x = 1;

			ValidateCursor();
		}
	}

	if ( c == L'{' || c == L'}' )
		CalcSmartTabs( cur.y, true );
	else
		OnTextChanged();

	m_bTopHighlightOnTyped = true;

	MakeTopFormatsDirty();

	AdvanceScrollbarOnDemand();
	
	OnACKeyTyped( c );
}

void CSmartText::OnMousePressed( MouseCode code )
{
	ResetBlinkTime();

	DestroyHoverTooltip();

	if ( m_pACMenu )
		DestroyACMenu();

	int x, y;
	input()->GetCursorPosition( x, y );
	ScreenToLocal( x, y );

	m_bIsRowSelecting = x < m_iTextInset;

	if ( code == MOUSE_LEFT )
		Selection_Reset();
	else
		m_bIsRowSelecting = false;

	if ( code == MOUSE_LEFT ||
		code == MOUSE_RIGHT && !Selection_Valid() )
	{
		PanelToCursor( x, y, cur );
		ValidateCursor();

		input()->SetMouseCapture( GetVPanel() );
		m_bIsSelecting = true;

		if ( m_bIsRowSelecting )
		{
			Selection_AddRow( cur.y );

			x = 0;
			y += FontTall();
			PanelToCursor( x, y, cur );
			ValidateCursor();
		}

		OnRowEnter( cur.y, m_bIsRowSelecting );
	}

	MakeTopFormatsDirty();

	ReleaseTooltipOnDemand();
}
void CSmartText::OnMouseDoublePressed( MouseCode code )
{
	if ( code != MOUSE_LEFT )
		return;

	int x, y;
	input()->GetCursorPosition( x, y );
	ScreenToLocal( x, y );

	Selection_Reset();
	PanelToCursor( x, y, cur );
	ValidateCursor();

	int x0 = hRows[cur.y]->GetWordLeft( cur.x );
	int x1 = hRows[cur.y]->GetWordRight( x0 );

	//int x1 = hRows[cur.y]->GetWordRight( cur.x );
	//int x0 = hRows[cur.y]->GetWordLeft( x1 );

	text_cursor sr;
	sr.y = cur.y;
	sr.x = x0;
	Selection_Select( sr );
	sr.x = x1;
	Selection_Select( sr );

	MoveCursorTo( sr );

	MakeTopFormatsDirty();
}
void CSmartText::OnMouseReleased( MouseCode code )
{
	input()->SetMouseCapture( NULL );
}
void CSmartText::OnMouseWheeled( int d )
{
	const bool ctrl = ( input()->IsKeyDown( KEY_LCONTROL ) || input()->IsKeyDown( KEY_RCONTROL ) );

	if ( ctrl )
		ChangeFontSize( d );
	else
		m_pScrollBar_V->SetValue( m_pScrollBar_V->GetValue() + d * -10 );
}
void CSmartText::OnCursorMoved( int x, int y )
{
	m_lLastCursorMoved = system()->GetTimeMillis();

	BaseClass::OnCursorMoved( x, y );
}
void CSmartText::OnMouseFocusTicked()
{
	int x, y;
	input()->GetCursorPosition( x, y );
	ScreenToLocal( x, y );
	SetCursor( ( x < m_iTextInset ) ? dc_up : dc_ibeam );

	//SetDragEnabled( x >= m_iTextInset && Selection_Valid() );

	BaseClass::OnMouseFocusTicked();
}
void CSmartText::OnCursorEntered()
{
	m_bAllowHoverTooltip = true;
	BaseClass::OnCursorEntered();
}
void CSmartText::OnCursorExited()
{
	m_bAllowHoverTooltip = false;
	BaseClass::OnCursorExited();
}
void CSmartText::OnMouseCaptureLost()
{
	BaseClass::OnMouseCaptureLost();

	m_bIsSelecting = false;
}
void CSmartText::OnThink()
{
	BaseClass::OnThink();

	if ( IsHistoryDirty() )
		OnHistoryDirty();
	else
	{
		if ( hHistory_Old.Count() )
			hHistory_Old[0]->cursor = cur;
	}

	if ( ParserHelper.HasDatabaseWaiting() )
	{
		delete m_pGlobalDatabase;
		m_pGlobalDatabase = ParserHelper.ObtainDatabase();

		AddEnvInfoToDB();
	}

	if ( ParserHelper.HasLiveDatabaseWaiting() && m_pGlobalDatabase )
	{
		CSmartObjectList *pLiveList = ParserHelper.ObtainLiveDatabase();
		m_pGlobalDatabase->MergeWithEntries( pLiveList );
		delete pLiveList;
	}

	if ( IsTopFormatsDirty() )
		UpdateTopFormats();

	UpdateParseCode();

	if ( m_bIsSelecting && input()->IsMouseDown( MOUSE_LEFT ) )
	{
		ResetBlinkTime();

		int sx, sy;
		GetSize( sx, sy );

		int x, y;
		input()->GetCursorPosition( x, y );
		ScreenToLocal( x, y );

		if ( m_bIsRowSelecting )
		{
			x = 0;
			text_cursor test;
			PanelToCursor( x, y, test );

			int numVRows = hRows[ test.y ]->GetRenderNumRows();

			if ( test.y == hRows.Count() - 1 )
				x = sx;

			if ( test.y >= select[0].y )
				y += FontTall() * numVRows;

			PanelToCursor( x, y, test );
		}

		PanelToCursor( x, y, cur );
		Selection_Select( cur );

		if ( Selection_Valid() )
			OnRowEnter(-1, true);

		int offsetDir = ( y > sy ) ? 1 : ( y < 0 ) ? -1 : 0;
		if ( y > sy )
			y -= sy;
		y = abs( y );

		int speed = max( 1, 60 - y ) * 8;

		if ( system()->GetTimeMillis() - scrollDelay > speed )
		{
			scrollDelay = system()->GetTimeMillis();
			m_pScrollBar_V->SetValue( m_pScrollBar_V->GetValue() + offsetDir );
		}

		if ( Selection_Valid() )
			MakeTopFormatsDirty();
	}

	CreateHoverTooltipThink();

	UpdateTooltipPositions();
}

void CSmartText::AddEnvInfoToDB()
{
	if ( !m_pGlobalDatabase )
		return;

	CSmartObject *pO = NULL;
	char tmp[MAX_PATH];
	int len = 0;

	if ( m_iEnvInfo & NODECUSTOM_ADDENV_LIGHTING_PS )
	{
		Q_snprintf( tmp, sizeof( tmp ), "#define %s", GetComboNameByID( HLSLCOMBO_NUM_LIGHTS ) );
		pO = new CSmartObject( ACOMP_DEFINE, tmp, GetComboNameByID( HLSLCOMBO_NUM_LIGHTS ) );
		Q_snprintf( tmp, sizeof( tmp ), "// Dynamic combo\n// Range: 0 - 4" );
		len = Q_strlen( tmp ) + 1;
		pO->m_pszHelptext = new char[len];
		Q_strcpy( pO->m_pszHelptext, tmp );
		AutoCopyStringPtr( IDENT_STATIC_LIVE_CODE, &pO->m_pszSourceFile );
		m_pGlobalDatabase->AddUnit( pO );

		pO = new CSmartObject( ACOMP_VAR, "const float3 g_cAmbientCube[6]", "g_cAmbientCube", "float3" );
		AutoCopyStringPtr( IDENT_STATIC_LIVE_CODE, &pO->m_pszSourceFile );
		m_pGlobalDatabase->AddUnit( pO );

		pO = new CSmartObject( ACOMP_VAR, "PixelShaderLightInfo g_cLightInfo[3]", "g_cLightInfo", "PixelShaderLightInfo" );
		AutoCopyStringPtr( IDENT_STATIC_LIVE_CODE, &pO->m_pszSourceFile );
		m_pGlobalDatabase->AddUnit( pO );
	}
	else if ( m_iEnvInfo & NODECUSTOM_ADDENV_LIGHTING_VS )
	{
		Q_snprintf( tmp, sizeof( tmp ), "#define %s", GetComboNameByID( HLSLCOMBO_LIGHT_STATIC ) );
		pO = new CSmartObject( ACOMP_DEFINE, tmp, GetComboNameByID( HLSLCOMBO_LIGHT_STATIC ) );
		Q_snprintf( tmp, sizeof( tmp ), "// Dynamic combo\n// Range: 0 - 1" );
		len = Q_strlen( tmp ) + 1;
		pO->m_pszHelptext = new char[len];
		Q_strcpy( pO->m_pszHelptext, tmp );
		AutoCopyStringPtr( IDENT_STATIC_LIVE_CODE, &pO->m_pszSourceFile );
		m_pGlobalDatabase->AddUnit( pO );

		Q_snprintf( tmp, sizeof( tmp ), "#define %s", GetComboNameByID( HLSLCOMBO_LIGHT_DYNAMIC ) );
		pO = new CSmartObject( ACOMP_DEFINE, tmp, GetComboNameByID( HLSLCOMBO_LIGHT_DYNAMIC ) );
		Q_snprintf( tmp, sizeof( tmp ), "// Dynamic combo\n// Range: 0 - 1" );
		len = Q_strlen( tmp ) + 1;
		pO->m_pszHelptext = new char[len];
		Q_strcpy( pO->m_pszHelptext, tmp );
		AutoCopyStringPtr( IDENT_STATIC_LIVE_CODE, &pO->m_pszSourceFile );
		m_pGlobalDatabase->AddUnit( pO );
	}
}

_smartRow *CSmartText::InsertRow( const wchar_t *text, int y )
{
	_smartRow *pRow = new _smartRow( m_Font, CalcInteriorWidth(), m_iTextInset );
	if ( text != NULL )
		pRow->InsertString( text, 0 );

	if ( y < 0 )
		hRows.AddToTail( pRow );
	else
		hRows.InsertBefore( y, pRow );

	OnTextChanged();
	return pRow;
}

_smartRow *CSmartText::InsertRow( const char *text, int y )
{
	wchar_t *pWStr = NULL;

	if ( text != NULL && *text )
	{
		int len = Q_strlen( text ) + 1;
		pWStr = new wchar_t[ len ];
		Q_UTF8ToUnicode( text, pWStr, len * sizeof(wchar_t) );
	}

	_smartRow *p = InsertRow( pWStr, y );

	delete [] pWStr;
	return p;
}
void CSmartText::ClearDynamicSpacing()
{
	for ( int i = 0; i < hRows.Count(); i++ )
	{
		if ( hRows[i]->IsDynamicSpacingEnabled() )
			hRows[i]->RemoveLeadingSpaces();
	}
	ValidateCursor();
}
int CSmartText::FindRow( _smartRow *p )
{
	if ( !p )
		return -1;

	int index = hRows.Find( p );
	if ( hRows.IsValidIndex( index ) )
		return index;

	return -1;
}
int CSmartText::GetLockedRow( int iNumLocked )
{
	int curLocked = 0;
	for ( int i = 0; i < hRows.Count(); i++ )
	{
		if ( hRows[i]->IsLocked() )
			curLocked++;
		if ( curLocked == iNumLocked )
			return i;
	}

	return -1;
}
void CSmartText::DeleteRow( int y )
{
	if ( y < 0 )
		y = hRows.Count() - 1;

	if ( y >= hRows.Count() )
		return;

	if ( !hRows[ y ]->IsDeletable() )
		return;

	delete hRows[ y ];
	hRows.Remove( y );

	MakeHistoryDirty();
}
void CSmartText::SetRowLocked( bool bLocked, int y )
{
	if ( y == -1 )
		y = hRows.Count() - 1;
	if ( y >= hRows.Count() ){ Assert(0); return; }
	hRows[y]->SetLocked( bLocked );
}
void CSmartText::SetRowDeletable( bool bDeletable, int y )
{
	if ( y == -1 )
		y = hRows.Count() - 1;
	if ( y >= hRows.Count() ){ Assert(0); return; }
	hRows[y]->SetDeletable( bDeletable );
}
bool CSmartText::IsRowLocked( int y )
{
	if ( y == -1 )
		y = hRows.Count() - 1;
	if ( y >= hRows.Count() ){ Assert(0); return false; }
	return hRows[y]->IsLocked();
}
bool CSmartText::IsRowDeletable( int y )
{
	if ( y == -1 )
		y = hRows.Count() - 1;
	if ( y >= hRows.Count() ){ Assert(0); return false; }
	return hRows[y]->IsDeletable();
}
bool CSmartText::SplitRow( text_cursor pos )
{
	pos.y = clamp( pos.y, 0, hRows.Count() - 1 );

	_smartRow *pNewRow = hRows[pos.y]->Split( pos.x );

	if ( !pNewRow )
		return false;

	hRows.InsertAfter(pos.y, pNewRow);

	OnTextChanged();
	return true;
}
void CSmartText::CalcSmartTabs( int y, bool bOnTyping )
{
	if ( y < 1 )
	{
		if ( bOnTyping )
			OnTextChanged();
		return;
	}

	int iIndent = 0;
	//int lastRow = min( bOnTyping ? y : ( y - 1 ), hRows.Count() - 1 );

	//bool bInComment = false;

	wchar_t *pszLastIndent = NULL; //hRows[y - 1]->CopyLeadingSpaces();

	text_cursor cw( bOnTyping ? cur.x : 0, y );

	GenerateCmtList();

	for (;;)
	{
		MoveCursor( -1, 0, false, false, &cw );

		bool bSkipLine = IsCommented( cw.x, cw.y );
		if ( !bSkipLine )
		{
			for ( int i = 0; i < hRows[cw.y]->Read().Count(); i++ )
			{
				const wchar_t &c = hRows[cw.y]->Read()[ i ];
				if ( c && !IS_SPACE( c ) )
				{
					if ( c == L'#' )
						bSkipLine = true;
					break;
				}
			}
		}

		if ( cw.x < hRows[cw.y]->Read().Count() &&
			!bSkipLine )
		{
			const wchar_t &c = Read(&cw); //hRows[cw.y]->Read()[cw.x];
			bool bEndParse = false;
			//bool bSearchScopeLine = false;

			text_cursor cw_orig( -1, -1 );

			if ( !c || IS_SPACE( c ) )
			{
			}
			else if ( ( c == L'{' && !bOnTyping ) )
			{
				iIndent++;
				bEndParse = true;
			}
			else
			{
				if ( c != L';' && c != L'{' && c != L'}' && IsHLSL() )
				{
					bool bArrayDef = false;
					int iscopecheck_intern = 0;
					if ( c == L',' )
					{
						text_cursor checkArray = cw;
						while ( MoveCursor( -1, 0, false, false, &checkArray ) )
						{
							if ( !CanRead( &checkArray ) )
								continue;
							if ( IsCommented( checkArray.x, checkArray.y ) )
								continue;

							const wchar_t &c_intern = Read(&checkArray);

							if ( c_intern == '(' )
							{
								iscopecheck_intern++;
							}
							else if ( c_intern == ')' )
							{
								iscopecheck_intern--;
							}
							else if ( c_intern == '{' )
							{
								if ( iscopecheck_intern == 0 )
									bArrayDef = true;
								break;
							}
							else if ( c_intern == ';' )
								break;
						}
					}

					if ( !bArrayDef )
					{
						iIndent++;
						cw_orig = cw;
					}
				}

				bEndParse = true;
				//bSearchScopeLine = true;

				if ( !bOnTyping || cw.x >= 1 && IS_SPACE( hRows[cw.y]->Read()[cw.x - 1] ) )
				{
					text_cursor bracketClose( cw );
					text_cursor bracketCurScope( -1, -1 );

					const bool bOnScopeClose = c == L'}';
					int iBracketCounter = ( bOnScopeClose ) ? -1 : 0;
					bool bScopeTestDirty = iBracketCounter != 0;

					for (;;)
					{
						MoveCursor( -1, 0, false, false, &bracketClose );

						bool bSkipLine = IsCommented( bracketClose.x, bracketClose.y );
						if ( !bSkipLine )
						{
							for ( int i = 0; i < hRows[bracketClose.y]->Read().Count(); i++ )
							{
								const wchar_t &c2 = hRows[bracketClose.y]->Read()[ i ];
								if ( c2 && !IS_SPACE( c2 ) )
								{
									if ( c2 == L'#' )
										bSkipLine = true;
									break;
								}
							}
						}

						if ( !bSkipLine && bracketClose.x < hRows[bracketClose.y]->Read().Count() )
						{
							const wchar_t &cB = Read(&bracketClose); //hRows[ bracketClose.y ]->Read()[ bracketClose.x ];

							if ( cB == L'{' )
							{
								iBracketCounter++;
								bScopeTestDirty = true;

								if ( bracketCurScope.x < 0 && bracketClose.y + 1 < hRows.Count() )
								{
									bracketCurScope = bracketClose;

									if ( c != L'}' )
										iIndent++;
								}
							}
							else if ( cB == L'}' )
							{
								iBracketCounter--;
								bScopeTestDirty = true;
							}

							if ( bScopeTestDirty && ( iBracketCounter >= 0 || iBracketCounter < 0 && !bOnScopeClose ) )
								break;
						}

						if ( bracketClose.x <= 0 && bracketClose.y <= 0 )
							break;
					}

					//if ( iBracketCounter >= 0 )
					{
						if ( bScopeTestDirty && iBracketCounter > 0 && bracketCurScope.y >= 0 )
							cw = bracketCurScope;
						else
							cw = bracketClose;
					}
				}
			}

			if ( bEndParse )
			{
				if ( cw_orig == cw )
					cw.x = cw.y = 0;

				pszLastIndent = hRows[cw.y]->CopyLeadingSpaces();
				break;
			}
		}

		if ( cw.x <= 0 && cw.y <= 0 )
			break;
	}

	KillCmtList();

	int oldLen = hRows[ y ]->GetTextLen();

	if ( bOnTyping )
		hRows[ y ]->RemoveLeadingSpaces();

	if ( pszLastIndent )
	{
		hRows[ y ]->InsertString( pszLastIndent, 0 );
		delete [] pszLastIndent;
	}

	for ( int i = 0; i < iIndent; i++ )
	{
		hRows[ y ]->PerformShiftTab();
	}

	int newLen = hRows[ y ]->GetTextLen();

	cur.x += newLen - oldLen;

	ValidateCursor();

	OnTextChanged();
}

void CSmartText::CursorToPanel( const text_cursor &c, int &x, int &y )
{
	x = 0;

	int rowStart = m_pScrollBar_V->GetValue();

#if DEBUG
	int numRowsAll = 0;
	for ( int i = 0; i < hRows.Count(); i++ )
		numRowsAll += hRows[ i ]->GetRenderNumRows();
	Assert( rowStart >= 0 && rowStart < numRowsAll );
#endif
	Assert( c.y >= 0 && c.y < hRows.Count() );

	int fontHeight = FontTall();

	y = fontHeight * -rowStart;

	for ( int i = 0; i < c.y; i++ )
		y += hRows[ i ]->GetRenderHeight();

	int rowmove_x, rowmove_y;
	hRows[ c.y ]->RelativeRowCursorToPanel( c.x, rowmove_x, rowmove_y );

	x += rowmove_x;
	y += rowmove_y;
}

void CSmartText::PanelToCursor( const int &x, const int &y, text_cursor &c )
{
	int fontHeight = FontTall();
	int virtualRow, realRow;
	CalcCurrentRenderRow( realRow, virtualRow );
	if ( realRow > 0 )
		virtualRow += hRows[realRow-1]->GetRenderNumRows();

	int curTestPos = fontHeight / 2 - ( virtualRow ) * fontHeight;
	int best_row_real = realRow;
	int best_row_virtual = virtualRow;
	int best_dist = 99999;

	bool bHitLastTime = false;

	for ( int i = max( 0, realRow - 1 ); i < hRows.Count(); i++ )
	{
		int curVirtualRows = hRows[i]->GetRenderNumRows();
		for ( int j = 0; j < curVirtualRows; j++ )
		{
			int cur_dist = abs( curTestPos - y );

			if ( cur_dist < best_dist )
			{
				best_dist = cur_dist;
				best_row_real = i;
				best_row_virtual = j;

				bHitLastTime = true;
			}
			else if ( bHitLastTime )
				i = hRows.Count();

			curTestPos += fontHeight;
		}
	}

	c.y = best_row_real;
	c.x = hRows[ c.y ]->PanelToCursorCalcX( best_row_virtual, x );
}
void CSmartText::MoveCursorTo( int x, int y )
{
	cur.y = clamp( y, 0, hRows.Count() - 1 );
	cur.x = clamp( x, 0, hRows[ y ]->GetTextLen() );
}
void CSmartText::MoveCursorTo( text_cursor c )
{
	MoveCursorTo( c.x, c.y );
}
bool CSmartText::MoveCursor( int leftright, int updown,
								bool bWordJump, bool bSelectRange, text_cursor *c )
{
	text_cursor &cw = c ? *c : cur;

	text_cursor startpos = cw;
	//int startposx = cw.x;

	if ( leftright != 0 )
	{
		Assert( cw.y >= 0 && cw.y < hRows.Count() );

		while ( leftright < 0 )
		{
			if ( cw.y == 0 && cw.x == 0 )
				break;

			cw.x--;

			if ( bWordJump && cw.x >= 0 )
			{
				if ( !CanRead( &cw ) || Read( &cw ) )
					cw.x = hRows[ cw.y ]->GetWordLeft( cw.x );
			}

			if ( cw.y > 0 && ( cw.x < 0 ||
				startpos.x == 0 && startpos.x == cw.x && bWordJump ) )
			{
				cw.y = cw.y - 1;
				cw.x = hRows[ cw.y ]->GetTextLen();
			}
			leftright++;
		}
		while ( leftright > 0 )
		{
			if ( cw.y == hRows.Count() - 1 && cw.x == hRows[ cw.y ]->GetTextLen() )
				break;

			if ( bWordJump )
				cw.x = hRows[ cw.y ]->GetWordRight( cw.x );
			else
				cw.x++;

			if ( cw.x > hRows[ cw.y ]->GetTextLen() ||
				startpos.x == cw.x && bWordJump )
			{
				cw.y = min( hRows.Count() - 1, cw.y + 1 );
				cw.x = 0;
			}
			leftright--;
		}

		if ( c == NULL )
			StompCursorHistory();
	}

	//while ( updown != 0 )
	if ( updown != 0 )
	{
		//if ( hRows[ cur.y ]->GetRenderNumRows() == 1 )
		//	cur.y

		int x, y;
		CursorToPanel( cw, x, y );
		y += FontTall() / 2 + FontTall() * updown;

		Assert( !c || c != &cur );

		x = c ? x : max( m_iXCursorHistory, x );
		if ( !c )
			m_iXCursorHistory = max( m_iXCursorHistory, x );

		PanelToCursor( x, y, cw );
	}

	if ( !bWordJump && bSelectRange && hRows[ cw.y ]->HasLinebreakAtCursor( cw ) )
		cw.x += ( startpos.x > cw.x ) ? -1 : 1;

	ValidateCursor( &startpos );

#if DEBUG
	text_cursor _check = startpos;
	ValidateCursor( &_check );
	Assert( _check == startpos );
#endif

	return startpos != cw;
}
void CSmartText::MoveCursorToEndOfRow( int y, text_cursor *c )
{
	if ( !c )
		c = &cur;

	if ( y < 0 )
		y = c->y;
	
	if ( y < 0 || y >= hRows.Count() )
		return;

	c->y = y;
	c->x = hRows[ c->y ]->GetTextLen();
}

void CSmartText::MoveCursorToEndOfText( text_cursor *c )
{
	if ( !c )
		c = &cur;

	c->y = hRows.Count() - 1;
	MoveCursorToEndOfRow( -1, c );
}

void CSmartText::OnTextChanged( int lastRealRow )
{
	StompCursorHistory();
	ValidateCursor();

	UpdateScrollbar( lastRealRow );
	OnSelectionChanged();

	//UpdateTopFormats();
	MakeCodeDirty();
}

void CSmartText::ValidateCursor( text_cursor *c )
{
	Assert( hRows.Count() > 0 );

	text_cursor &cw = ( c != NULL ) ? *c : cur;

	cw.y = clamp( cw.y, 0, hRows.Count() - 1 );
	cw.x = clamp( cw.x, 0, hRows[ cw.y ]->GetTextLen() );
}

int CSmartText::FontTall()
{
	return surface()->GetFontTall( m_Font );
}

void CSmartText::StompCursorHistory()
{
	m_iXCursorHistory = 0;
}

void CSmartText::AdvanceScrollbarOnDemand()
{
	int x,y;
	CursorToPanel( cur, x, y );
	int sx, sy;
	GetSize( sx, sy );

	int fontTall = FontTall();

	int ydelta = 0;
	if ( y < 0 )
		ydelta = abs( y ) / fontTall * -1;
	else if ( y + fontTall >= sy )
		ydelta = ( y - sy + fontTall ) / fontTall + 1;

	if ( ydelta == 0 )
		return;

	m_pScrollBar_V->SetValue( m_pScrollBar_V->GetValue() + ydelta );
}

void CSmartText::UpdateSelectionFormat()
{
	delete [] m_pszLastSelectedWord;
	m_pszLastSelectedWord = NULL;

	if ( Selection_Active() && select[0] != select[1] )
	{
		int x0, x1;
		text_cursor sr[2];
		Selection_GetSorted( sr );

		if ( sr[0].y == sr[1].y && sr[0] < sr[1] )
		{
			text_cursor href_min = sr[0];
			text_cursor href_max = sr[1];
			href_max.x -= 1;

			text_cursor href_bounds_0 = href_min;
			text_cursor href_bounds_1 = href_max;
			href_bounds_0.x--;
			href_bounds_1.x++;

			hRows[href_bounds_0.y]->ToVirtualCursor(href_bounds_0);
			hRows[href_bounds_1.y]->ToVirtualCursor(href_bounds_1);

			if ( (!CanRead(&href_bounds_0) || !IS_CHAR(Read(&href_bounds_0)) && !IS_NUMERICAL(Read(&href_bounds_0)) ) &&
				(!CanRead(&href_bounds_1) || !IS_CHAR(Read(&href_bounds_1)) && !IS_NUMERICAL(Read(&href_bounds_1)) ) )
			{
				m_pszLastSelectedWord = hRows[href_min.y]->ReadInterval( href_min.x, href_max.x, false, true );
				wchar_t *pwalkref = m_pszLastSelectedWord;
				while ( *pwalkref )
				{
					if ( !IS_CHAR(*pwalkref) &&
						!IS_NUMERICAL(*pwalkref) )
					{
						pwalkref = NULL;
						break;
					}
					pwalkref++;
				}

				if ( pwalkref != NULL )
					for ( int i = 0; i < hRows.Count(); i++ )
						hRows[i]->SetHighlightRef(m_pszLastSelectedWord);
			}
		}

		for ( int i = sr[0].y; i <= min( sr[1].y, hRows.Count() - 1 ); i++ )
		{
			if ( !Selection_GetRowRange( i, x0, x1 ) )
				continue;

			_inlineFormat *f = new _inlineFormat();
			f->bOverride_fg = true;
			f->bOverride_bg = true;
			f->col_fg = _col_SelectionFG;
			f->col_bg = _col_SelectionBG;
			f->start = x0;
			f->end = x1;

			if ( sr[1].y > i )
				f->bCRBlock = true;

			hRows[i]->AddHighlightFormat( f );
		}
	}
}

void CSmartText::UpdateHighlightFormats()
{
	for ( int i = 0; i < hRows.Count(); i++ )
	{
		hRows[ i ]->ClearHighlightFormats();
	}

	UpdateCmtFormat();
	UpdateSelectionFormat();
}

void CSmartText::MakeTopFormatsDirty()
{
	m_bTopFormatsDirty = true;;
}
bool CSmartText::IsTopFormatsDirty()
{
	return m_bTopFormatsDirty;
}
void CSmartText::UpdateTopFormats( bool bDoHighlights )
{
	m_bTopFormatsDirty = false;

	if ( m_bTopHighlightLock )
		return;

	for ( int i = 0; i < hRows.Count(); i++ )
		hRows[ i ]->ClearTopFormats();

	if ( !bDoHighlights || Selection_Valid() )
		return;

	text_cursor bracketHighlight = cur;

	if ( m_bTopHighlightOnTyped )
	{
		m_bTopHighlightOnTyped = false;
		MoveCursor( -1, 0, false, false, &bracketHighlight );
	}

	if ( !hRows[ bracketHighlight.y ]->Read().Count() ||
		bracketHighlight.x >= hRows[ bracketHighlight.y ]->Read().Count() )
		return;

	const wchar_t &ref = Read(&bracketHighlight); //hRows[ bracketHighlight.y ]->Read()[ bracketHighlight.x ];
	const int iDir = IS_BRACKET_OPENING( ref ) ? 1 : IS_BRACKET_CLOSING( ref ) ? -1 : 0;

	if ( !iDir )
		return;

	int iBracketCounter = iDir;

	text_cursor bracketHighlight_0 = bracketHighlight;

	m_bTopHighlightLock = true;

	for (;;)
	{
		MoveCursor( iDir, 0, false, false, &bracketHighlight );

		bool bValid = true;

		if ( bracketHighlight.x < 0 && bracketHighlight.y < 0 )
			bValid = false;

		if ( bracketHighlight.x >= hRows[ bracketHighlight.y ]->GetTextLen() && bracketHighlight.y >= hRows.Count() - 1 )
			bValid = false;

		if ( bValid && CanRead( &bracketHighlight ) )
		{
			const wchar_t &cur = Read(&bracketHighlight); //hRows[ bracketHighlight.y ]->Read()[ bracketHighlight.x ];

			if ( IS_BRACKET_SAME_TYPE( ref, cur ) )
			{
				iBracketCounter += IS_BRACKET_OPENING( cur ) ? 1 : -1;
			}
		}

		if ( !iBracketCounter )
			break;

		if ( !bValid || bracketHighlight.x <= 0 && bracketHighlight.y <= 0 )
		{
			iBracketCounter = -1;
			break;
		}
	}

	m_bTopHighlightLock = false;

	if ( iBracketCounter == 0 )
	{
		hRows[bracketHighlight_0.y]->ToRealCursor( bracketHighlight_0 );
		hRows[bracketHighlight.y]->ToRealCursor( bracketHighlight );

		_inlineFormat *f = new _inlineFormat();
		f->bOverride_bg = true;
		f->col_bg = ST_COL_HIGHLIGHT_BRACKET;
		f->start = bracketHighlight_0.x;
		f->end = bracketHighlight_0.x + 1;
		hRows[ bracketHighlight_0.y ]->AddTopFormat( f );

		f = new _inlineFormat( *f );
		f->start = bracketHighlight.x;
		f->end = bracketHighlight.x + 1;
		hRows[ bracketHighlight.y ]->AddTopFormat( f );
	}
}

void CSmartText::ProcessDelete( bool bAfter )
{
	if ( Selection_Valid() )
	{
		text_cursor rg[2];
		Selection_GetSorted( rg );

		hRows[ rg[0].y ]->ToVirtualCursor( rg[0] );
		MoveCursorTo( rg[0] );
		Selection_Delete();
	}
	else
	{
		//text_cursor orig = cur;

		hRows[ cur.y ]->ToRealCursor( cur );
		hRows[ cur.y ]->KillLineBreaks();

		int currowLen = hRows[ cur.y ]->GetTextLen();

		if ( bAfter && cur.x < currowLen ||
			!bAfter && cur.x > 0 )
		{
			if ( !hRows[ cur.y ]->IsLocked() )
			{
				MakeHistoryDirty();
				hRows[ cur.y ]->DeleteChar( cur.x, bAfter );
			}
		}

		wchar_t *append = NULL;

		if ( !bAfter && cur.x == 0 && cur.y > 0 )
		{
			if ( hRows[ cur.y ]->IsDeletable() )
			{
				if ( !hRows[ cur.y ]->IsDynamicSpacingEnabled() )
					append = hRows[ cur.y ]->ReadInterval();

				DeleteRow( cur.y );

				MoveCursorToEndOfRow( cur.y - 1 );
			}
		}
		else if ( bAfter && cur.x == currowLen && cur.y < hRows.Count() - 1 )
		{
			if ( hRows[ cur.y + 1 ]->IsDeletable() )
			{
				if ( !hRows[ cur.y + 1 ]->IsDynamicSpacingEnabled() )
					append = hRows[ cur.y + 1 ]->ReadInterval();

				DeleteRow( cur.y + 1 );

				hRows[ cur.y ]->UpdateLinebreaks();
				MoveCursorToEndOfRow();
			}
		}
		else
		{
			if ( !bAfter && cur.x > 0 )
				MoveCursor( -1 );

			hRows[ cur.y ]->ToVirtualCursor( cur );
		}

		if ( append != NULL )
		{
			int oldVRows = hRows[ cur.y ]->GetRenderNumRows( cur.x );
			hRows[ cur.y ]->InsertString( append );
			int newVRows = hRows[ cur.y ]->GetRenderNumRows( cur.x );
			MoveCursor( newVRows - oldVRows );

			delete [] append;
		}
	}

	OnTextChanged();

	MakeTopFormatsDirty();
}
void CSmartText::ProcessCut()
{
	ValidateCursor();

	ProcessCopy();

	if ( Selection_Valid() )
	{
		text_cursor rg[2];
		Selection_GetSorted( rg );

		hRows[ rg[0].y ]->ToVirtualCursor( rg[0] );
		MoveCursorTo( rg[0] );
		Selection_Delete();
	}
	else if ( hRows.Count() > 1 )
	{
		if ( hRows[ cur.y ]->IsDeletable() )
		{
			DeleteRow( cur.y );
			ValidateCursor();
		}
	}

	OnTextChanged();

	MakeTopFormatsDirty();
}
void CSmartText::ProcessCopy()
{
	ValidateCursor();

	if ( Selection_Valid() )
		Selection_Copy();
	else
	{
		wchar_t *pRow = hRows[ cur.y ]->ReadInterval( -1, -1, true, true );
		int wStrLen = Q_wcslen( pRow ) + 2;
		wchar_t *CRRow = new wchar_t[ wStrLen ];

		// wasted 4 hours tracking down this crash. comment for memorial!
		Q_memset( CRRow, 0, wStrLen * sizeof(wchar_t) ); // takes bytes, not array length, BRO

		CRRow[ 0 ] = CR_HACK_CHAR;
		CRRow++;
		Q_wcsncpy( CRRow, pRow, (wStrLen-1) * sizeof( wchar_t ) ); // Uhhhhhhhh
		CRRow--;

		delete [] pRow;
		system()->SetClipboardText( CRRow, Q_wcslen( CRRow ) ); 
		delete [] CRRow;
	}

	OnTextChanged();
}
void CSmartText::ProcessPaste()
{
	Selection_Delete();

	int strLen = system()->GetClipboardTextCount();
	wchar_t *buf = new wchar_t[ strLen + 1 ];
	Q_memset( buf, 0, sizeof( wchar_t ) * strLen );
	system()->GetClipboardText( 0, buf, sizeof( wchar_t ) * strLen );

	if ( buf && Q_wcslen(buf) )
		MakeHistoryDirty();

	ParseInsertText( buf );
	delete [] buf;

	OnTextChanged();
	AdvanceScrollbarOnDemand();
}

void CSmartText::OnRowEdit( int y )
{
	if ( y < 0 )
		y = cur.y;

	if ( hRows.IsValidIndex( y ) && hRows[y]->IsDynamicSpacingEnabled() )
	{
		hRows[y]->SetDynamicSpacingEnabled( false );

		if ( cur.x < hRows[y]->GetTextLen() )
			hRows[y]->RemoveLeadingSpaces();
	}
}

void CSmartText::OnRowEnter( int y, bool bNoTabs )
{
	if ( y < 0 )
		y = cur.y;

	const bool bEmpty = !hRows[y]->GetTextLen();
	const bool bIsDynamic = hRows[y]->IsDynamicSpacingEnabled();

	if ( !hRows.IsValidIndex( y ) || ( !bIsDynamic && !bEmpty ) )
		return;

	if ( bEmpty )
		hRows[y]->SetDynamicSpacingEnabled( true );

	int oldCursorX = m_iXCursorHistory;

	hRows[y]->RemoveLeadingSpaces();

	if ( !bNoTabs )
		CalcSmartTabs(y);

	m_iXCursorHistory = oldCursorX;
	ValidateCursor();
}

void CSmartText::ParseInsertText( const wchar_t *str )
{
	OnRowEdit();

	ValidateCursor();

	const wchar_t *walk = str;

	while ( walk && *walk )
	{
		//bool bAdvance_x = true;

		if ( *walk == L'\r' )
		{
		}
		else if ( *walk == CR_HACK_CHAR )
		{
			cur.x = 0;
			//bAdvance_x = false;
		}
		else if ( *walk == L'\n' )
		{
			if ( SplitRow( cur ) )
			{
				hRows[ cur.y ]->OnTextChanged();

				//hRows[ cur.y ]->OnTextChanged();
				//MoveCursorToEndOfRow();

				//MoveCursor( 1 );
				//bAdvance_x = false;
				cur.x = 0;
				cur.y++;
				ValidateCursor();
			}
		}
		else
		{
			const wchar_t *findEndRow = walk;

			while ( findEndRow && *findEndRow &&
				*findEndRow != CR_HACK_CHAR &&
				*findEndRow != L'\n' &&
				*findEndRow != L'\r' )
				findEndRow++;

			bool bHaltOnWcntrl = findEndRow && *findEndRow;

			int numMoved_Str = findEndRow - walk; // - ( *findEndRow ? 1 : 0 );
			int numMoved_Row = bHaltOnWcntrl ? numMoved_Str : hRows[ cur.y ]->GetTextLen();

			//hRows[ cur.y ]->InsertString( walk, cur.x, numMoved_Str - (bHaltOnWcntrl?1:0), true );
			hRows[ cur.y ]->InsertString( walk, cur.x, numMoved_Str, true );

			//hRows[ cur.y ]->KillLineBreaks();
			//hRows[ cur.y ]->UpdateLinebreaks();
			hRows[ cur.y ]->ToRealCursor( cur );

			if ( !bHaltOnWcntrl )
			{
				numMoved_Row = hRows[ cur.y ]->GetTextLen() - numMoved_Row;
			}

			hRows[ cur.y ]->OnTextChanged();

			//else
			//	cur.x = hRows[ cur.y ]->GetTextLen();

			cur.x += numMoved_Row;
			hRows[ cur.y ]->ToVirtualCursor( cur );

			walk = findEndRow - 1;

			//hRows[ cur.y ]->InsertChar( *walk, cur.x, true );
		}

		//if ( bAdvance_x )
		//	cur.x++;
		walk++;
	}

	hRows[ cur.y ]->OnTextChanged();
}

void CSmartText::ParseInsertText( const char *str )
{
	if ( !str || !*str )
		return;

	int len = Q_strlen( str ) + 1;
	wchar_t *pW = new wchar_t[len];
	Q_UTF8ToUnicode( str, pW, sizeof(wchar_t) * len );

	ParseInsertText(pW);

	delete [] pW;
}

bool CSmartText::Selection_GetRowRange( const int &row, int &x_min, int &x_max,
										text_cursor *cMin, text_cursor *cMax )
{
	Assert( row >= 0 && row < hRows.Count() );
	text_cursor sr[2];

	if ( !cMin || !cMax )
		Selection_GetSorted( sr );
	else
	{
		Assert( (*cMin) <= (*cMax) );

		sr[0] = *cMin;
		sr[1] = *cMax;
	}

	if ( sr[0].y > row || sr[1].y < row )
		return false;

	if ( sr[0].y == row )
	{
		x_min = sr[0].x;
		if ( sr[1].y == row )
			x_max = sr[1].x;
		else
			x_max = hRows[ row ]->GetTextLen( true );
	}
	else if ( sr[1].y == row )
	{
		x_min = 0;
		x_max = sr[1].x;
	}
	else
	{
		x_min = 0;
		x_max = hRows[ row ]->GetTextLen( true );
	}

	return true;
}
void CSmartText::OnSelectionChanged()
{
	for ( int i = 0; i < 2; i++ )
	{
		select[i].y = clamp( select[i].y, 0, hRows.Count() - 1 );
		select[i].x = clamp( select[i].x, 0, hRows[ select[i].y ]->GetTextLen() );
	}

	UpdateHighlightFormats();
}
void CSmartText::Selection_Reset()
{
	bool bWasValid = m_bSelectionValid;
	m_bSelectionValid = false;

	for ( int i = 0; i < 2; i++ )
		select[i].x = select[i].y = 0;

	if ( bWasValid )
		OnSelectionChanged();
}
void CSmartText::Selection_Select( text_cursor pos )
{
	Assert( pos.y >= 0 && pos.y < hRows.Count() );

	hRows[ pos.y ]->ToRealCursor( pos );

	if ( !Selection_Active() )
		select[0] = select[1] = pos;
	else
		select[1] = pos;

	m_bSelectionValid = true;

	OnSelectionChanged();
}
void CSmartText::Selection_AddRow( int y )
{
	Assert( y >= 0 && y < hRows.Count() );

	text_cursor tmp;
	tmp.y = y;
	tmp.x = 0;

	Selection_Select( tmp );
	tmp.x = hRows[y]->GetTextLen();
	Selection_Select( tmp );
}
void CSmartText::Selection_Delete()
{
	if ( Selection_Active() && select[0] != select[1] )
	{
		int x0, x1;
		text_cursor sr[2];
		Selection_GetSorted( sr );

		MoveCursorTo( sr[0] );

		wchar_t *append = NULL;

		for ( int i = hRows.Count() - 1; i >= 0; i-- )
		{
			if ( !Selection_GetRowRange( i, x0, x1 ) )
				continue;

			hRows[ i ]->KillLineBreaks();

			if ( !append &&
				x0 == 0 &&
				x1 < hRows[ i ]->GetTextLen( true ) &&
				sr[0].y < i )
			{
				append = hRows[ i ]->ReadInterval( x1 );
			}

			if ( sr[ 0 ].y < i )
			{
				if ( hRows[ i ]->IsDeletable() )
				{
					DeleteRow( i );
				}
				else if ( !hRows[ i ]->IsLocked() )
					hRows[ i ]->DeleteInterval();
				else
					hRows[ i ]->UpdateLinebreaks();
			}
			else
			{
				hRows[ i ]->DeleteInterval( x0, x1 - 1 );
			}
		}

		if ( append )
		{
			hRows[ sr[0].y ]->InsertString( append );

			delete [] append;
		}

		ValidateCursor();

		MakeHistoryDirty();
	}

	Selection_Reset();
}
void CSmartText::Selection_Copy( text_cursor *cMin, text_cursor *cMax, wchar_t **pBuffOut, bool bAddCR )
{
#if DEBUG
	if ( cMin == NULL && cMax == NULL )
		Assert( Selection_Valid() );
#endif

	int x0, x1;
	CUtlVector< wchar_t* >hRowStream;

	text_cursor sr[2];

	if ( !cMin || !cMax )
		Selection_GetSorted( sr );
	else
	{
		if ( cMax->y < 0 )
			cMax->y = hRows.Count() - 1;
		if ( cMax->x < 0 )
			MoveCursorToEndOfRow( -1, cMax );

		Assert( (*cMin) <= (*cMax) );

		sr[0] = *cMin;
		sr[1] = *cMax;
	}

	if ( sr[0] >= sr[1] )
	{
		if ( pBuffOut )
			*pBuffOut = NULL;
		return;
	}

	for ( int i = 0; i < hRows.Count(); i++ )
	{
		if ( !Selection_GetRowRange( i, x0, x1, &sr[0], &sr[1] ) )
			continue;

		wchar_t *rowStr = NULL;

		bool bAllowLF = i < sr[1].y;

		if ( sr[0].y == sr[1].y && i == sr[0].y )
			x1 -= 1;

		if ( sr[1].y == i && sr[1].x == 0 )
		{
			if ( bAllowLF )
			{
				rowStr = new wchar_t[2];
				rowStr[0] = L'\n';
				rowStr[1] = L'\0';
			}
		}
		else
			rowStr = hRows[i]->ReadInterval( x0, x1, bAllowLF, true );

		if ( rowStr )
			hRowStream.AddToTail( rowStr );
	}

	int wholeBuffLen = 1;
	for ( int i = 0; i < hRowStream.Count(); i++ )
		wholeBuffLen += Q_wcslen( hRowStream[ i ] );

	if ( bAddCR )
		wholeBuffLen += hRowStream.Count() - 1;

	wchar_t *pszBuff = new wchar_t[ wholeBuffLen ];
	wchar_t *pWriter = pszBuff;
	Q_memset( pszBuff, 0, sizeof( wchar_t ) * wholeBuffLen );

	for ( int i = 0; i < hRowStream.Count(); i++ )
	{
		int rowLen = Q_wcslen( hRowStream[ i ] );
		Q_memcpy( pWriter, hRowStream[ i ], sizeof( wchar_t ) * rowLen );
		pWriter += rowLen;

		if ( bAddCR && i < hRowStream.Count() - 1 )
		{
			pWriter--;
			*pWriter = '\r';
			pWriter++;
			*pWriter = '\n';
			pWriter++;
		}
	}

	for ( int i = 0; i < hRowStream.Count(); i++ )
		delete [] hRowStream[ i ];
	hRowStream.Purge();

	//pWriter--;
	*pWriter = L'\0';

	if ( pBuffOut != NULL )
		*pBuffOut = pszBuff;
	else
	{
		system()->SetClipboardText( pszBuff, wholeBuffLen - 1 ); //Q_wcslen( pszBuff ) );
		delete [] pszBuff;
	}
}
void CSmartText::Selection_GetSorted( text_cursor *v )
{
	text_cursor &a = v[0];
	text_cursor &b = v[1];

	a = select[0];
	b = select[1];

	if ( select[0] > select[1] )
		swap( a, b );
}

void CSmartText::KillCmtList()
{
	m_hCmtList.Purge();
}

void CSmartText::GenerateCmtList()
{
	KillCmtList();

	CUtlVector< wchar_t >*phReader;

	_cmtCtrl MultilineCmt;
	bool bInMultilineCmt = false;

	for ( int y = 0; y < hRows.Count(); y++ )
	{
		phReader = &hRows[ y ]->Read();

		if ( bInMultilineCmt )
		{
			MultilineCmt = _cmtCtrl( y, 0, -1 );
		}

		int readerPos = 0;

		for ( int x = 0; x < phReader->Count() - 1; x++ )
		{
			const wchar_t &c = phReader->Element( x );

			if ( !c )
				continue;

			if ( bInMultilineCmt )
			{
				if ( c == L'*' && phReader->Element( x + 1 ) == L'/' )
				{
					MultilineCmt._x1 = readerPos + 1;
					m_hCmtList.AddToTail( MultilineCmt );

					bInMultilineCmt = false;
				}
			}
			else if ( c == L'/' && phReader->Element( x + 1 ) == L'/' )
			{
				m_hCmtList.AddToTail( _cmtCtrl( y, readerPos, -1 ) );

				//y++;
				break;
			}
			else if ( c == L'/' && phReader->Element( x + 1 ) == L'*' )
			{
				MultilineCmt = _cmtCtrl( y, readerPos, -1 );
				bInMultilineCmt = true;
			}

			readerPos++;
		}

		if ( bInMultilineCmt )
		{
			m_hCmtList.AddToTail( MultilineCmt );

			//bInMultilineCmt = false;
		}
	}
}

bool CSmartText::IsCommented( int x, int y )
{
	for ( int i = 0; i < m_hCmtList.Count(); i++ )
		if ( m_hCmtList[i].IsCommented( x, y ) )
			return true;

	return false;
}

void CSmartText::UpdateCmtFormat()
{
	GenerateCmtList();

	for ( int i = 0; i < m_hCmtList.Count(); i++)
	{
		_inlineFormat *f = new _inlineFormat();
		f->bOverride_fg = true;
		f->col_fg = ST_COL_CMT_FG;
		f->start = m_hCmtList[i]._x0;
		f->end = m_hCmtList[i]._x1;

		if ( m_hCmtList[i]._x1 >= 0 )
			f->end++;

		hRows[ m_hCmtList[i]._y ]->AddHighlightFormat( f );
	}

	KillCmtList();
}


void CSmartText::CreateACMenu( int iTooltipMode )
{
	if ( !IsACEnabled() )
		return;

	const bool bFunctionTooltip = iTooltipMode == ACTOOLTIP_FUNC;
	const bool bObjectTooltip = iTooltipMode == ACTOOLTIP_OBJECT;
	const bool bManualMode = !bFunctionTooltip && !bObjectTooltip;

	DestroyACMenu(true, !bObjectTooltip);

	if ( !m_pGlobalDatabase || m_pGlobalDatabase->GetNumEntries() < 1 )
	{
		OnACMenuCreationFailed();
		return;
	}

	text_cursor action_start = cur;

	//if ( !CanRead( &action_start ) && action_start.x > 0 )
	if ( action_start.x > 0 )
		action_start.x--;

	GetWordLeft( action_start );

	// blah.
	// blah(
	// blah (
	// blah ( dagfdag, agdag,
	// datatype blah ( sgsgj		// NOT
	// sdghg; sdgsdgh

	// only go left?


	bool bObjectAccess = false;
	bool bFunctionParams = false;
	bool bExpressionStart = false;

	int moves = 0;
	int bracketScope = 0;

	do
	{
		if ( !CanRead( &action_start ) )
			continue;

		const wchar_t &c = Read( &action_start );

		if ( bFunctionTooltip )
		{
			if ( c == L')' )
				bracketScope--;
			else if ( c == L'(' )
				bracketScope++;
			else if ( bFunctionTooltip && bracketScope > 0 && !IS_SPACE( c ) )
				bFunctionParams = true;
			else if ( c == L';' )
				bExpressionStart = true;
		}
		else if ( bObjectTooltip )
		{
			if ( !moves && c == L'.' )
				bObjectAccess = true;
			//else if ( bObjectAccess && !IS_CHAR( c ) && !IS_NUMERICAL( c ) )
			//	break;
			else if ( c == L';' )
				bExpressionStart = true;
		}
		else
		{
			bExpressionStart = true;
		}

		moves++;

	} while ( !bObjectAccess && !bFunctionParams && !bExpressionStart &&
		MoveCursor( -1, 0, false, false, &action_start ) );

	//const bool bOriginSameRow = action_start.y == cur.y;
	char *pMasterVar = NULL;

	//bFunctionParams = bFunctionParams && bOriginSameRow;

	if ( bFunctionParams )
	{
		GetWordLeft( action_start );
	}

	text_cursor menuPosRef = ( bFunctionParams || bObjectAccess ) ? action_start : cur;


	if ( bFunctionTooltip )
	{
		m_curACStart = menuPosRef;

		if ( !bFunctionParams || !CanRead( &m_curACStart ) || !m_pGlobalDatabase )
			return;

		if ( !IS_CHAR( Read( &m_curACStart ) ) )
			return;

		Assert( !m_pFuncTooltip );

		int x0 = m_curACStart.x;
		int x1 = hRows[m_curACStart.y]->GetWordRight( x0 ) - 1;

		if ( x1 < x0 )
			return;

		wchar_t *snippet = hRows[m_curACStart.y]->ReadInterval( x0, x1, false, true );
		char cSnippet[MAX_PATH];

		if ( !snippet )
			return;

		Q_UnicodeToUTF8( snippet, cSnippet, sizeof(cSnippet) );

		int iscopedepth, iscopenum, textPos;
		GetScopeData( m_curACStart, iscopedepth, iscopenum, textPos );

		CSmartObject *pOFunc = NULL;
		for ( int i = 0; i < m_pGlobalDatabase->GetNumEntries(); i++ )
		{
			CSmartObject *pO = m_pGlobalDatabase->GetEntry(i);

			if ( pO->m_iType != ACOMP_FUNC )
				continue;
			if ( !pO->m_pszIdentifier )
				continue;
			if ( !pO->IsObjectVisible( textPos, iscopedepth, iscopenum, IDENT_LIVE_CODE ) )
				continue;

			if ( Q_strcmp( pO->m_pszIdentifier, cSnippet ) )
				continue;

			pOFunc = pO;
		}

		if ( pOFunc != NULL )
		{
			m_pFuncTooltip = new CSmartTooltip( this, "functooltip" );
			m_pFuncTooltip->Init( pOFunc, CSmartTooltip::STTIPMODE_FUNCTIONPARAMS );

			UpdateTooltipPositions();
		}

		delete [] snippet;

		return;
	}
	else if ( bManualMode )
	{
		text_cursor test = menuPosRef;
		test.x--;

		if ( GetWordLeft( test ) )
			menuPosRef = test;
	}
	else if ( bObjectTooltip )
	{
		if ( !bObjectAccess )
			return;

		menuPosRef.x++;
		text_cursor wordStart = action_start;
		wordStart.x--;

		text_cursor checkArray( wordStart );
		do
		{
			if ( !CanRead( &checkArray ) )
				continue;
			const wchar_t &c = Read( &checkArray );

			if ( c == '[' )
			{
				checkArray.x--;
				if ( CanRead( &checkArray ) )
				{
					wordStart = checkArray;
					action_start = wordStart;
					action_start.x++;
				}
			}
			else if ( IS_SPACE(c) || c == '.' )
				break;

		} while( MoveCursor( -1, 0, false, false, &checkArray ) );

		if ( !GetWordLeft( wordStart ) )
			return;

		if ( !CanRead( &wordStart ) )
			return;

		wchar_t *pWMasterVar = hRows[wordStart.y]->ReadInterval( wordStart.x, action_start.x - 1, false, true );
		if ( pWMasterVar && *pWMasterVar )
		{
			int len = Q_wcslen( pWMasterVar ) + 1;
			pMasterVar = new char[ len ];
			Q_UnicodeToUTF8( pWMasterVar, pMasterVar, len );
		}
		delete [] pWMasterVar;
	}


	text_cursor save_ac_start = m_curACStart;
	m_curACStart = menuPosRef;

	int iscopedepth, iscopenum, textPos;
	GetScopeData( m_curACStart, iscopedepth, iscopenum, textPos );


	m_pACMenu = new CSmartAutocomplete( this, "acmenu" );
	m_pACMenu->SetFont( __hfcInstance.GetFont( __hfcInstance.fSizeMin() + __hfcInstance.fStepSize() * 2 ) );
	m_pACMenu->SetMenuItemHeight( m_pACMenu->GetMenuItemHeight() - 2 );
	m_pACMenu->SetNumberOfVisibleItems( 10 );

	bool bTryObject = bObjectTooltip && bObjectAccess;

	bool bSuccess = m_pACMenu->InitList( m_pGlobalDatabase, textPos, iscopedepth, iscopenum,
		IDENT_LIVE_CODE, bTryObject ? pMasterVar : NULL );

	if ( bManualMode && CanRead( &menuPosRef ) )
	{
		int x0 = menuPosRef.x;
		int x1 = hRows[menuPosRef.y]->GetWordRight( x0 );

		bool bRead = x1 >= x0;

		wchar_t *snippet = bRead ? hRows[menuPosRef.y]->ReadInterval( x0, x1 - 1 ) : NULL;
		m_pACMenu->MatchInput( snippet );
		delete [] snippet;
	}

	delete [] pMasterVar;


	if ( !bSuccess )
	{
		m_curACStart = save_ac_start;
		DestroyACMenu( true, !bObjectTooltip );
		return;
	}

	int pX, pY, mX, mY;
	m_pACMenu->InvalidateLayout( true );
	m_pACMenu->GetSize( mX, mY );

	CalcACPos( menuPosRef, pX, pY, mX, mY );
	m_pACMenu->SetPos( pX, pY );
	m_pACMenu->SetKeyBoardInputEnabled( false );

	m_pACMenu->RequestFocus();
}
void CSmartText::CreateHoverTooltipThink()
{
	if ( !IsACEnabled() )
		return;

	if ( m_lLastCursorMoved <= 0.0l ||
		!m_bAllowHoverTooltip )
	{
		DestroyHoverTooltip();
		return;
	}

	if ( m_pHoverTooltip != NULL && m_pHoverTooltip->IsVisible() )
		return;

	long delta = system()->GetTimeMillis() - m_lLastCursorMoved;
	if ( delta <= 600 )
		return;

	DestroyHoverTooltip();

	int mx, my;
	text_cursor curHover;
	input()->GetCursorPosition( mx, my );
	ScreenToLocal( mx, my );
	PanelToCursor( mx, my, curHover );

	if ( !CanRead(&curHover) || !m_pGlobalDatabase )
		return;

	if ( !IS_CHAR(Read(&curHover)) && !IS_NUMERICAL(Read(&curHover)) )
		return;

	m_curHoverMin = curHover;
	GetWordLeft( m_curHoverMin );
	ValidateCursor( &m_curHoverMin );

	int x0 = m_curHoverMin.x;
	int x1 = hRows[m_curHoverMin.y]->GetWordRight( x0 ) - 1;

	if ( x1 < x0 )
		return;

	wchar_t *snippet = hRows[m_curHoverMin.y]->ReadInterval( x0, x1, false, true );

	m_curHoverMax = text_cursor( x1 + 1, m_curHoverMin.y );

	if ( !snippet )
		return;

	int len = Q_wcslen( snippet ) + 1;
	char *pCStr = new char[ len ];
	Assert( pCStr );
	Q_UnicodeToUTF8( snippet, pCStr, len );

	int iscopedepth, iscopenum, textPos;
	GetScopeData( curHover, iscopedepth, iscopenum, textPos );

	CSmartObject *pO = NULL;
	for ( int i = 0; i < m_pGlobalDatabase->GetNumEntries() /*&& !pO*/; i++ )
	{
		CSmartObject *pCur = m_pGlobalDatabase->GetEntry(i);

		if ( !pCur->m_pszIdentifier || Q_strcmp( pCur->m_pszIdentifier, pCStr ) )
			continue;

		if ( !pCur->IsObjectVisible( textPos, iscopedepth, iscopenum, IDENT_LIVE_CODE ) )
			continue;

		if ( pO && pO->m_iScopeDepth >= pCur->m_iScopeDepth )
			continue;

		if ( pO && pO->m_iScopeNum > pCur->m_iScopeNum )
			continue;

		pO = pCur;
	}

	delete [] snippet;
	delete [] pCStr;

	if ( !pO )
		return;

	m_pHoverTooltip = new CSmartTooltip( this, "hovertooltip" );
	m_pHoverTooltip->Init( pO );

	UpdateTooltipPositions();
}
void CSmartText::DestroyACMenu( bool bMenu, bool bTooltip )
{
	if ( bMenu )
	{
		if ( m_pACMenu )
			m_pACMenu->MarkForDeletion();
		m_pACMenu = NULL;
	}

	if ( bTooltip )
	{
		if ( m_pFuncTooltip )
			m_pFuncTooltip->MarkForDeletion();
		m_pFuncTooltip = NULL;
	}
}
void CSmartText::DestroyHoverTooltip()
{
	if ( m_pHoverTooltip )
	{
		m_pHoverTooltip->MarkForDeletion();
		m_lLastCursorMoved = 0.0l;
		m_pHoverTooltip = NULL;
	}
}
void CSmartText::CalcACPos( const text_cursor &c, int &x, int &y, int menuSizeX, int menuSizeY )
{
	int sX, sY;
	CursorToPanel( c, x, y );
	LocalToScreen( x, y );
	surface()->GetScreenSize( sX, sY );

	if ( y + FontTall() + menuSizeY <= sY )
		y += FontTall();
	else
		y -= menuSizeY;

	if ( x + menuSizeX > sX )
		x -= x + menuSizeX - sX;
	else
		x += 2;
}
void CSmartText::OnACMenuCreationFailed()
{
}
bool CSmartText::OnACCatchKeyCode( KeyCode code )
{
	if ( !m_pACMenu || !m_pACMenu->IsVisible() )
	{
		ReleaseTooltipOnDemand();
		return false;
	}

	if ( code == KEY_UP || code == KEY_DOWN )
	{
		m_pACMenu->MoveAlongMenuItemList( (code == KEY_UP) ? Menu::MENU_UP : Menu::MENU_DOWN, 0, true );
		return true;
	}
	else if ( code == KEY_ENTER )
	{
		InsertFromAC();
		return true;
	}
	else if ( code == KEY_SPACE || code == KEY_PERIOD )
	{
		text_cursor findWord = cur;
		findWord.x--;

		wchar_t *snippet = NULL;

		if ( CanRead( &findWord ) && IS_CHAR( Read( &findWord ) ) )
			if ( GetWordLeft( findWord ) && CanRead( &findWord ) )
				snippet = hRows[ findWord.y ]->ReadInterval( findWord.x, cur.x - 1, false, true );

		if ( snippet && m_pACMenu->IsExactMatch( snippet, true ) >= 0 )
			InsertFromAC();
		else
			DestroyACMenu();

		delete [] snippet;
	}
	else if ( code == KEY_LEFT || code == KEY_RIGHT ||
		code == KEY_SPACE || code == KEY_ESCAPE )
	{
		DestroyACMenu();
	}
	else if ( code == KEY_BACKSPACE )
	{
		if ( cur <= m_curACStart )
			DestroyACMenu();
	}

	return false;
}
void CSmartText::InsertFromAC( const char *pIdent )
{
	bool bOwnsMemory = pIdent == NULL;

	if ( !pIdent )
		pIdent = m_pACMenu->AllocHighlightItemName();

	if ( pIdent )
	{
		if ( CanRead(&m_curACStart) &&
			( IS_CHAR(Read(&m_curACStart)) || IS_NUMERICAL(Read(&m_curACStart)) )
			)
		{
			text_cursor cRight = m_curACStart;
			cRight.x = hRows[cRight.y]->GetWordRight( cRight.x ) - 1;

			if ( cRight.x >= m_curACStart.x )
				hRows[m_curACStart.y]->DeleteInterval( m_curACStart.x, cRight.x );
		}

		text_cursor rep = m_curACStart;
		//text_cursor repPre = rep;
		//repPre.x--;

		//if ( !CanRead( &rep ) && CanRead( &repPre ) )
		//{
		//	wchar_t chPre = Read( &repPre );

		//	if ( IS_NUMERICAL(chPre) || IS_CHAR(chPre) )
		//		rep = repPre;
		//}

		//if ( CanRead(&rep) //&&
		//	//( IS_CHAR( Read(&rep) ) || IS_NUMERICAL( Read(&rep) ) )
		//	)
		//{
		//	GetWordLeft( rep );

		//	if ( CanRead(&rep) && !IS_SPACE( Read(&rep) ) //&&
		//		//( IS_CHAR( Read(&rep) ) || IS_NUMERICAL( Read(&rep) ) )
		//		)
		//	{
		//		int iRight = hRows[rep.y]->GetWordRight( rep.x ) - 1;
		//		if ( iRight >= rep.x )
		//			hRows[rep.y]->DeleteInterval( rep.x, iRight );
		//	}
		//}

		//int lenOld = hRows[rep.y]->GetTextLen();

		cur = rep;
		hRows[rep.y]->ToRealCursor( cur );
		hRows[rep.y]->InsertString( pIdent, rep.x );
		cur.x += Q_strlen( pIdent );
		hRows[rep.y]->ToVirtualCursor( cur );

		ValidateCursor();

		if ( bOwnsMemory )
			delete [] pIdent;

		OnRowEdit( cur.y );
		MakeHistoryDirty();
	}
	DestroyACMenu();

}
bool CSmartText::OnACKeyTyped( wchar_t c )
{
	if ( c == '.' )
	{
		CreateACMenu( ACTOOLTIP_OBJECT );
		return false;
	}
	else if ( c == '(' || c == ',' || c == ')' )
	{
		CreateACMenu( ACTOOLTIP_FUNC );
		ReleaseTooltipOnDemand();
		return false;
	}

	if ( !m_pACMenu || !m_pACMenu->IsVisible() )
	{
		ReleaseTooltipOnDemand();
		return false;
	}

	if ( c && !IS_CHAR( c ) && !IS_NUMERICAL( c ) )
	{
		DestroyACMenu();
		return false;
	}

	text_cursor wleft = cur;
	wleft.x--;
	GetWordLeft( wleft );
	ValidateCursor( &wleft );

	int x0 = wleft.x;
	int x1 = hRows[wleft.y]->GetWordRight( x0 );

	bool bRead = x1 >= x0;

	wchar_t *snippet = bRead ? hRows[wleft.y]->ReadInterval( x0, x1 - 1 ) : NULL;

	m_pACMenu->MatchInput( snippet );

	int ACResult = m_pACMenu->IsExactMatch( snippet, false );
	if ( ACResult >= 0 )
	{
		char tmp[MAX_PATH];
		m_pACMenu->GetItemText( ACResult, tmp, sizeof( tmp ) );
		InsertFromAC( tmp );
	}

	delete [] snippet;

	return false;
}

void CSmartText::ReleaseTooltipOnDemand()
{
	if ( ShouldReleaseTooltip() )
		DestroyACMenu();
}

bool CSmartText::ShouldReleaseTooltip()
{
	if ( !m_pFuncTooltip || !m_pFuncTooltip->IsVisible() )
		return false;

	if ( cur <= m_curACStart )
		return true;

	bool bExpressionChange = false;

	GenerateCmtList();

	text_cursor scopeTest = cur;
	int iScope = 0;
	bool bHitBracket = false;
	int iCountCommas = 0;

	while ( MoveCursor( -1, 0, false, false, &scopeTest ) && m_curACStart < scopeTest )
	{
		if ( IsCommented( scopeTest.x, scopeTest.y ) )
			continue;

		if ( !CanRead(&scopeTest) )
			continue;

		const wchar_t &ch = Read( &scopeTest );

		switch ( ch )
		{
		case L'(':
			iScope++;
			bHitBracket = true;
			break;
		case L')':
			iScope--;
			bHitBracket = true;
			break;
		case L';':
			bExpressionChange = true;
			break;
		case L',':
			if ( iScope > 0 )
			{
				iScope = 0;
				iCountCommas = 0;
			}
			if ( iScope == 0 )
				iCountCommas++;
			break;
		}
	}

	KillCmtList();

	if ( bExpressionChange ||
		iScope == 0 && bHitBracket )
		return true;

	m_pFuncTooltip->MoveToFront();
	m_pFuncTooltip->SetActiveParam( bHitBracket ? iCountCommas + 1 : 0 );
	return false;
}

void CSmartText::UpdateTooltipPositions()
{
	if ( m_pFuncTooltip && m_pFuncTooltip->IsVisible() )
	{
		int pX, pY, mX, mY;

		CursorToPanel( cur, pX, pY );
		CursorToPanel( m_curACStart, mX, mY );
		int yDelta = pY - mY;

		if ( m_pFuncTooltip->IsLayoutInvalid() )
			m_pFuncTooltip->InvalidateLayout( true );

		m_pFuncTooltip->GetSize( mX, mY );
		CalcACPos( m_curACStart, pX, pY, mX, mY );
		m_pFuncTooltip->SetPos( max( 0, pX ), pY + yDelta );
	}

	if ( m_pHoverTooltip && m_pHoverTooltip->IsVisible() )
	{
		int mx, my;
		text_cursor curHover;
		input()->GetCursorPosition( mx, my );
		ScreenToLocal( mx, my );
		PanelToCursor( mx, my, curHover );

		if ( curHover < m_curHoverMin || curHover > m_curHoverMax )
			DestroyHoverTooltip();
		else
		{
			int pX, pY, mX, mY;
			CursorToPanel( m_curHoverMin, pX, pY );

			if ( m_pHoverTooltip->IsLayoutInvalid() )
				m_pHoverTooltip->InvalidateLayout( true );

			m_pHoverTooltip->GetSize( mX, mY );
			CalcACPos( m_curHoverMin, pX, pY, mX, mY );
			m_pHoverTooltip->SetPos( max(0, pX), pY );
		}
	}
}
bool CSmartText::GetWordLeft( text_cursor &c )
{
	//text_cursor wordStart = action_start;

	bool bMoved = false;

	while ( CanRead( &c ) )
	{
		const wchar_t &ch = Read( &c );

		if ( !( IS_CHAR( ch ) || IS_NUMERICAL( ch ) ) )
			break;

		c.x--;
		bMoved = true;
	}

	if ( bMoved )
		c.x++;

	return bMoved;
	//action_start = wordStart;
}

void CSmartText::OnMenuItemSelected( Panel *panel )
{
	MenuItem *pItem = assert_cast< MenuItem* >( panel );
	Assert( pItem );

	if ( !pItem )
		return;

	if ( m_pACMenu && ipanel()->HasParent( pItem->GetVPanel(), m_pACMenu->GetVPanel() ) )
	{
		char buf[MAX_PATH];
		pItem->GetText( buf, sizeof(buf) );
		InsertFromAC(buf);
	}
}

void CSmartText::MakeCodeDirty()
{
	m_bCodeDirty = true;
}
bool CSmartText::IsCodeDirty()
{
	return m_bCodeDirty;
}
void CSmartText::UpdateParseCode()
{
	if ( !IsLiveParseEnabled() )
		return;

	if ( !IsCodeDirty() )
		return;

	if ( gpGlobals->curtime - m_flLastCodeParse < 1.0f )
		return;

	m_flLastCodeParse = gpGlobals->curtime;

	if ( !m_pGlobalDatabase )
		return;

	if ( !hRows.Count() )
		return;

	text_cursor cu_0( 0, 0 );
	text_cursor cu_1( hRows[hRows.Count()-1]->GetTextLen(true), hRows.Count() - 1 );

	if ( cu_1 <= cu_0 )
	{
		m_pGlobalDatabase->PurgeUnitsOfSameSource( IDENT_LIVE_CODE );
		return;
	}

	m_bCodeDirty = false;

	wchar_t *pBuff = NULL;

	Selection_Copy( &cu_0, &cu_1, &pBuff );

	if ( pBuff == NULL )
		return;

	ParserThread.RequestUpdate( PARSERREQUEST_USER, pBuff, new CSmartObjectList( *m_pGlobalDatabase ) );

	//int len = Q_wcslen( pBuff ) + 1;

	//if ( len > 1 )
	//{
	//	char *pCStrBuff = new char[ len ];

	//	if ( pCStrBuff != NULL )
	//	{
	//		CUtlBuffer buf( pCStrBuff, len );
	//		Q_UnicodeToUTF8( pBuff, pCStrBuff, len );

	//		m_pGlobalDatabase->ParseCode_Stream( buf, false, NULL, IDENT_LIVE_CODE, Q_strlen(pCStrBuff) - 1 );
	//		buf.Clear();

	//		delete [] pCStrBuff;
	//	}
	//}

	//delete [] pBuff;
}

bool CSmartText::CanRead( text_cursor *c )
{
	text_cursor &cw = ( c != NULL ) ? *c : cur;
	return cw.y >= 0 && cw.y < hRows.Count() && cw.x >= 0 && cw.x < hRows[cw.y]->Read().Count();
}

const wchar_t &CSmartText::Read( text_cursor *c )
{
	text_cursor &cw = ( c != NULL ) ? *c : cur;
	Assert( CanRead( &cw ) );

	return hRows[cw.y]->Read()[cw.x];
}

void CSmartText::GetScopeData( text_cursor c, int &iScopeDepth, int &iScopeNum, int &iTextPos )
{
	iScopeDepth = iScopeNum = iTextPos = 0;
	text_cursor cread(0,0);

	if ( c <= cread )
		return;

	for ( int i = 0; i < c.y; i++ )
		iTextPos += hRows[i]->GetTextLen( true ) + 1;
	iTextPos += c.x;

	GenerateCmtList();

	CUtlVector< int >hScopeCounter;
	hScopeCounter.AddToTail(0);

	while ( cread < c )
	{
		if ( !IsCommented( cread.x, cread.y ) && CanRead( &cread ) )
		{
			const wchar_t &ch = Read( &cread );

			if ( ch == L'{' )
			{
				iScopeDepth++;

				if ( iScopeDepth >= hScopeCounter.Count() )
					hScopeCounter.AddToTail( -1 );

				Assert( iScopeDepth < hScopeCounter.Count() );
				hScopeCounter[ iScopeDepth ]++;
			}
			else if ( ch == L'}' )
			{
				iScopeDepth--;
			}
		}

		MoveCursor( 1, 0, false, false, &cread );
	}

	Assert( iScopeDepth < hScopeCounter.Count() );
	iScopeNum = hScopeCounter[ iScopeDepth ];

	hScopeCounter.Purge();

	KillCmtList();

	//wchar_t *pBuff = NULL;
	//Selection_Copy( &cmin, &c, &pBuff );

	//if ( !pBuff )
	//	return;


	//delete [] pBuff;
}

bool CSmartText::IsHistoryDirty()
{
	return m_bHistoryDirty;
}

void CSmartText::MakeHistoryDirty()
{
	m_bHistoryDirty = true;
}

void CSmartText::OnHistoryDirty()
{
	m_bHistoryDirty = false;
	PushHistory();

	m_iActiveHistoryIndex = sm_iHistoryIndex;
	sm_iHistoryIndex++;

	PostActionSignal( new KeyValues( "CodeChanged" ) );
}

HHISTORYIDX CSmartText::GetCurrentHistoryIndex()
{
	return m_iActiveHistoryIndex;
}

void CSmartText::FlushHistory( bool bOld /*= true*/, bool bNew /*= true*/ )
{
	if ( bOld )
		hHistory_Old.PurgeAndDeleteElements();
	if ( bNew )
		hHistory_New.PurgeAndDeleteElements();
}

void CSmartText::PushHistory( int dir /*= 1*/ )
{
	Assert( dir != 0 );

	if ( dir > 0 )
	{
		if ( dir < 2 )
			FlushHistory(false);

		__sTextHistory *pEntry = CreateHistoryEntry();
		hHistory_Old.AddToHead( pEntry );

		while ( hHistory_Old.Count() > MAX_SMARTEDIT_HISTORY )
		{
			delete hHistory_Old[ MAX_SMARTEDIT_HISTORY ];
			hHistory_Old.Remove( MAX_SMARTEDIT_HISTORY );
		}
	}
	else
	{
		if ( !hHistory_Old.Count() )
			return;

		hHistory_New.AddToHead( hHistory_Old[0] );
		hHistory_Old.Remove( 0 );
	}
}

void CSmartText::PopHistory( int dir /*= 1*/ )
{
	Assert( dir != 0 );

	if ( dir > 0 )
	{
		if ( hHistory_Old.Count() < 2 )
			return;

		ApplyHistoryEntry( hHistory_Old[1] );
		PushHistory(-1);
	}
	else
	{
		if ( hHistory_New.Count() < 1 )
			return;

		ApplyHistoryEntry( hHistory_New[0] );
		PushHistory(2);

		delete hHistory_New[0];
		hHistory_New.Remove(0);
	}
}

__sTextHistory *CSmartText::CreateHistoryEntry()
{
	Assert( hRows.Count() );
	__sTextHistory *out = new __sTextHistory( sm_iHistoryIndex );

	ValidateCursor();

	for ( int i = 0; i < hRows.Count(); i++ )
		out->rows.AddToTail( new _smartRow( *hRows[i] ) );
	out->cursor = cur;

	return out;
}

void CSmartText::ApplyHistoryEntry(__sTextHistory *history )
{
	if ( history->rows.Count() )
	{
		hRows.PurgeAndDeleteElements();

		for ( int i = 0; i < history->rows.Count(); i++ )
			hRows.AddToTail( new _smartRow( *history->rows[i] ) );
	}

	cur = history->cursor;
	m_iActiveHistoryIndex = history->iIndex;
	ValidateCursor();

	OnTextChanged();
	OnLayoutUpdated();

	AdvanceScrollbarOnDemand();

	PostActionSignal( new KeyValues( "CodeChanged" ) );
}
