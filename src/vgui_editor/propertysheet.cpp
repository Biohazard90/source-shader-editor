//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ToolWindow.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/PropertyPage.h>
#include "vgui_controls/AnimationController.h"

#include "vgui_editor_platform.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

namespace vgui
{

class ContextLabel : public Label
{
	DECLARE_CLASS_SIMPLE( ContextLabel, Label );
public:

	ContextLabel( Button *parent, char const *panelName, char const *text ):
		BaseClass( (Panel *)parent, panelName, text ),
		m_pTabButton( parent )
	{
		SetBlockDragChaining( true );
	}

	virtual void OnMousePressed( MouseCode code )
	{
		if ( m_pTabButton )
		{
			m_pTabButton->FireActionSignal();
		}
	}

	virtual void OnMouseReleased( MouseCode code )
	{
		BaseClass::OnMouseReleased( code );

		if ( GetParent() )
		{
			GetParent()->OnCommand( "ShowContextMenu" );
		}
	}

	virtual void ApplySchemeSettings( IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		HFont marlett = pScheme->GetFont( "Marlett" );
		SetFont( marlett );
		SetTextInset( 0, 0 );
		SetContentAlignment( Label::a_northwest );

		if ( GetParent() )
		{
			SetFgColor( pScheme->GetColor( "Button.TextColor", GetParent()->GetFgColor() ) );
			SetBgColor( GetParent()->GetBgColor() );
		}
	}
private:

	Button	*m_pTabButton;
};

//-----------------------------------------------------------------------------
// Purpose: Helper for drag drop
// Input  : msglist - 
// Output : static PropertySheet
//-----------------------------------------------------------------------------
static PropertySheet *IsDroppingSheet( CUtlVector< KeyValues * >& msglist )
{
	if ( msglist.Count() == 0 )
		return NULL;

	KeyValues *data = msglist[ 0 ];
	PropertySheet *sheet = reinterpret_cast< PropertySheet * >( data->GetPtr( "propertysheet" ) );
	if ( sheet )
		return sheet;

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: A single tab
//-----------------------------------------------------------------------------
class PageTab : public Button
{
	DECLARE_CLASS_SIMPLE( PageTab, Button );

private:
	bool _active;
	Color _textColor;
	Color _dimTextColor;
	int m_bMaxTabWidth;
	IBorder *m_pActiveBorder;
	IBorder *m_pNormalBorder;
	PropertySheet	*m_pParent;
	Panel			*m_pPage;
	ImagePanel		*m_pImage;
	char			*m_pszImageName;
	bool			m_bShowContextLabel;
	bool			m_bAttemptingDrop;
	ContextLabel	*m_pContextLabel;
	long			m_hoverActivatePageTime;
	long			m_dropHoverTime;
	Button			*m_pButtonClose;
	bool			m_bShouldShowCloseButton;
	bool			m_bDragEnabled;

	Color	m_ColBgColor;
	bool	m_bDrawBg;

	bool	m_bClosing;
	bool	m_bOpening;
	float	m_flCurSizeX;

#define __ptab_button_inset 3

	int m_iDesignatedWidth;
	int m_iLastMouseX;

	//double flAnimTime;

//#define __ptab_ANIMDUR 0.5f

public:

	PageTab(PropertySheet *parent, const char *panelName, const char *text, char const *imageName, int maxTabWidth, Panel *page, bool showContextButton,
		long hoverActivatePageTime = -1, bool bCloseButton = false ) : 
		Button( (Panel *)parent, panelName, text),
		m_pParent( parent ),
		m_pPage( page ),
		m_pImage( 0 ),
		m_pszImageName( 0 ),
		m_bShowContextLabel( showContextButton ),
		m_bAttemptingDrop( false ),
		m_hoverActivatePageTime( hoverActivatePageTime ),
		m_dropHoverTime( -1 )
	{
		SetCommand(new KeyValues("TabPressed"));
		_active = false;
		m_bDrawBg = false;
		m_bMaxTabWidth = maxTabWidth;
		//SetDropEnabled( true );
		//SetDragEnabled( m_pParent->IsDraggableTab() );
		m_bDragEnabled = m_pParent->IsDraggableTab();

		m_bClosing = false;
		m_bOpening = bCloseButton;
		m_flCurSizeX = 0;
		m_iDesignatedWidth = 0;
		m_iLastMouseX = 0;

		//if ( m_bOpening )
		//	flAnimTime = system()->GetCurrentTime() + __ptab_ANIMDUR;

		if ( imageName )
		{
			m_pImage = new ImagePanel( this, text );
			int buflen = Q_strlen( imageName ) + 1;
			m_pszImageName = new char[ buflen ];
			Q_strncpy( m_pszImageName, imageName, buflen );

		}

		m_pActiveBorder = NULL;
		m_pNormalBorder = NULL;

		SetMouseClickEnabled( MOUSE_RIGHT, true );
		m_pContextLabel = m_bShowContextLabel ? new ContextLabel( this, "Context", "9" ) : NULL;

		REGISTER_COLOR_AS_OVERRIDABLE( _textColor, "selectedcolor" );
		REGISTER_COLOR_AS_OVERRIDABLE( _dimTextColor, "unselectedcolor" );

		m_bShouldShowCloseButton = true;

		if ( bCloseButton )
			m_pButtonClose = new Button( this, "closebutton", "x", this, "close_tab" );
		else
			m_pButtonClose = NULL;
	}

	~PageTab()
	{
		delete[] m_pszImageName;
	}

	const int GetDesignatedWidth()
	{
		int x, y;
		GetSize(x,y);

		return ( m_iDesignatedWidth > 0 ) ? m_iDesignatedWidth : x;
	};

	bool IsAnimating()
	{
		return m_bClosing || m_bOpening;
	}

	void CloseTab()
	{
		m_bClosing = true;
		m_bOpening = false;

		if ( m_iDesignatedWidth > 0 )
			m_flCurSizeX = m_iDesignatedWidth;
		//flAnimTime = system()->GetCurrentTime() + __ptab_ANIMDUR;
	}

	void SetText( const char *pszText )
	{
		BaseClass::SetText( pszText );
		//InvalidateLayout(true);
		//m_flCurSizeX = m_iDesignatedWidth;
	}

	bool IsClosing()
	{
		return m_bClosing;
	}

	void SetCloseButtonVisible( bool bVisible )
	{
		m_bShouldShowCloseButton = bVisible;
	}

	void SetCustomBgColor( Color c )
	{
		m_ColBgColor = c;
		m_bDrawBg = true;
		InvalidateLayout();
	};

	virtual void Paint()
	{
		if ( m_bDrawBg )
		{
			surface()->DrawSetColor( m_ColBgColor ); //GetBgColor() );
			int sx, sy;
			GetSize(sx,sy);
			surface()->DrawFilledRect( 0, 0, sx, sy );
		}
		BaseClass::Paint();
	}

	virtual void OnCursorEntered()
	{
		m_dropHoverTime = system()->GetTimeMillis();
	}

	virtual void OnCursorExited()
	{
		m_dropHoverTime = -1;
	}

	virtual void OnThink()
	{
		if ( m_bAttemptingDrop && m_hoverActivatePageTime >= 0 && m_dropHoverTime >= 0 )
		{
			long hoverTime = system()->GetTimeMillis() - m_dropHoverTime;
			if ( hoverTime > m_hoverActivatePageTime )
			{
				FireActionSignal();
				SetSelected(true);
				Repaint();
			}
		}
		m_bAttemptingDrop = false;

		if ( m_bDragEnabled && input()->GetMouseCapture() == GetVPanel() )
		{
			if ( m_pParent->DoScroll() )
				OnDoDrag();
		}

		BaseClass::OnThink();
	}

	virtual bool IsDroppable( CUtlVector< KeyValues * >&msglist )
	{
		m_bAttemptingDrop = true;

		if ( !GetParent() )
			return false;

		PropertySheet *sheet = IsDroppingSheet( msglist );
		if ( sheet )
			return GetParent()->IsDroppable( msglist );

		return BaseClass::IsDroppable( msglist );
	}

	virtual void OnDroppablePanelPaint( CUtlVector< KeyValues * >& msglist, CUtlVector< Panel * >& dragPanels )
	{
		PropertySheet *sheet = IsDroppingSheet( msglist );
		if ( sheet )
		{
			Panel *target = GetParent()->GetDropTarget( msglist );
			if ( target )
			{
			// Fixme, mouse pos could be wrong...
				target->OnDroppablePanelPaint( msglist, dragPanels );
				return;
			}
		}

		// Just highlight the tab if dropping onto active page via the tab
		BaseClass::OnDroppablePanelPaint( msglist, dragPanels );
	}

	virtual void OnPanelDropped( CUtlVector< KeyValues * >& msglist )
	{
		PropertySheet *sheet = IsDroppingSheet( msglist );
		if ( sheet )
		{
			Panel *target = GetParent()->GetDropTarget( msglist );
			if ( target )
			{
			// Fixme, mouse pos could be wrong...
				target->OnPanelDropped( msglist );
			}
		}

		// Defer to active page...
		Panel *active = m_pParent->GetActivePage();
		if ( !active || !active->IsDroppable( msglist ) )
			return;

		active->OnPanelDropped( msglist );
	}

	virtual void OnDragFailed( CUtlVector< KeyValues * >& msglist )
	{
		PropertySheet *sheet = IsDroppingSheet( msglist );
		if ( !sheet )
			return;

		// Create a new property sheet
		if ( m_pParent->IsDraggableTab() )
		{
			if ( msglist.Count() == 1 )
			{
				KeyValues *data = msglist[ 0 ];
                int screenx = data->GetInt( "screenx" );
				int screeny = data->GetInt( "screeny" );

				// m_pParent->ScreenToLocal( screenx, screeny );
				if ( !m_pParent->IsWithin( screenx, screeny ) )
				{
					Panel *page = reinterpret_cast< Panel * >( data->GetPtr( "propertypage" ) );
					PropertySheet *sheet = reinterpret_cast< PropertySheet * >( data->GetPtr( "propertysheet" ) );
					char const *title = data->GetString( "tabname", "" );
					if ( !page || !sheet )
						return;
					
					// Can only create if sheet was part of a ToolWindow derived object
					ToolWindow *tw = dynamic_cast< ToolWindow * >( sheet->GetParent() );
					if ( tw )
					{
						IToolWindowFactory *factory = tw->GetToolWindowFactory();
						if ( factory )
						{
							bool hasContextMenu = sheet->PageHasContextMenu( page );
							sheet->RemovePage( page );
							factory->InstanceToolWindow( tw->GetParent(), sheet->ShouldShowContextButtons(), page, title, hasContextMenu );

							if ( sheet->GetNumPages() == 0 )
							{
								tw->MarkForDeletion();
							}
						}
					}
				}
			}
		}
	}

	virtual void OnCreateDragData( KeyValues *msg )
	{
		Assert( m_pParent->IsDraggableTab() );

		msg->SetPtr( "propertypage", m_pPage );
		msg->SetPtr( "propertysheet", m_pParent );
		char sz[ 256 ];
		GetText( sz, sizeof( sz ) );
		msg->SetString( "tabname", sz  );
		msg->SetString( "text", sz );
	}

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		// set up the scheme settings
		Button::ApplySchemeSettings(pScheme);

		_textColor = GetSchemeColor("PropertySheet.SelectedTextColor", GetFgColor(), pScheme);
		_dimTextColor = GetSchemeColor("PropertySheet.TextColor", GetFgColor(), pScheme);
		m_pActiveBorder = pScheme->GetBorder("TabActiveBorder");
		m_pNormalBorder = pScheme->GetBorder("TabBorder");
	}

	void PerformLayout()
	{
		BaseClass::PerformLayout();

		int tabHeight = m_pParent->GetTabHeight();
		int maxWidth;
		int maxHeight;

		if ( m_pImage )
		{
			ClearImages();
			m_pImage->SetImage(scheme()->GetImage(m_pszImageName, false));
			AddImage( m_pImage->GetImage(), 2 );
			int w, h;
			m_pImage->GetSize( w, h );
			w += m_pContextLabel ? 10 : 0;
			if ( m_pContextLabel )
			{
				m_pImage->SetPos( 10, 0 );
			}

			maxWidth = w + 4;
			maxHeight = h + 2;
		}
		else
		{
			int wide, tall;
			int contentWide, contentTall;
			GetSize(wide, tall);
			GetContentSize(contentWide, contentTall);

			wide = max(m_bMaxTabWidth, contentWide + 10);  // 10 = 5 pixels margin on each side
			wide += m_pContextLabel ? 10 : 0;
			if ( m_pButtonClose && m_bShouldShowCloseButton )
				wide += tabHeight - 7;

			maxWidth = wide;
			maxHeight = tall;
		}

		if ( IsAnimating() )
		{
			int goal = m_bClosing ? 0 : maxWidth;

			if ( abs( goal - m_flCurSizeX ) > 1 )
				m_flCurSizeX = Approach( goal, m_flCurSizeX, m_pParent->m_dFrametime * 1000.0f * ((float)maxWidth/200.0f) );
			else
				m_flCurSizeX = goal;
			//m_flCurSizeX = Lerp( clamp( 1.0f - (system()->GetCurrentTime() - flAnimTime)/__ptab_ANIMDUR, 0, 1 ), 

			if ( m_bClosing && m_flCurSizeX < 1 )
			{
				m_bClosing = false;
				RemovePageAndTab();
				return;
			}
			else if ( m_bOpening && abs( m_flCurSizeX - maxWidth ) < 1 )
			{
				m_bOpening = false;
				m_flCurSizeX = maxWidth;
			}

			SetSize( m_flCurSizeX, maxHeight );
		}
		else
			SetSize( maxWidth, maxHeight );

		m_iDesignatedWidth = maxWidth;

		if ( m_pButtonClose )
		{
			m_pButtonClose->SetSize( tabHeight - __ptab_button_inset * 2, tabHeight - __ptab_button_inset * 2 );
			m_pButtonClose->SetVisible( m_bShouldShowCloseButton );
			m_pButtonClose->SetTextInset( 7, 0 );

			if ( m_bDrawBg )
			{
				Color cBG = Color( max( 0, m_ColBgColor.r() - 15 ),
								max( 0, m_ColBgColor.g() - 15 ),
								max( 0, m_ColBgColor.b() - 15 ),
								m_ColBgColor.a() );
				m_pButtonClose->SetDefaultColor( GetFgColor(), cBG );
				m_pButtonClose->SetPaintBackgroundEnabled( true );
				m_pButtonClose->SetPaintBackgroundType( 0 );
			}
		}

		if ( m_pContextLabel )
		{
			SetTextInset( 12, 0 );
			int w, h;
			GetSize( w, h );
			m_pContextLabel->SetBounds( 0, 0, 10, h );
		}
	}

	virtual void OnSizeChanged(int newWide, int newTall)
	{
		BaseClass::OnSizeChanged(newWide,newTall);

		if ( m_pButtonClose && m_pParent )
		{
			int tabHeight = m_pParent->GetTabHeight();
			m_pButtonClose->SetPos( newWide - tabHeight + __ptab_button_inset, __ptab_button_inset );
		}
	}

	virtual void ApplySettings( KeyValues *inResourceData )
	{
		BaseClass::ApplySettings(inResourceData);

		const char *pBorder = inResourceData->GetString("activeborder_override", "");
		if (*pBorder)
		{
			m_pActiveBorder = scheme()->GetIScheme(GetScheme())->GetBorder( pBorder );
		}
		pBorder = inResourceData->GetString("normalborder_override", "");
		if (*pBorder)
		{
			m_pNormalBorder = scheme()->GetIScheme(GetScheme())->GetBorder( pBorder );
		}
	}

	virtual void OnCommand( char const *cmd )
	{
		if ( !Q_stricmp( cmd, "ShowContextMenu" ) )
		{
			KeyValues *kv = new KeyValues("OpenContextMenu");
			kv->SetPtr( "page", m_pPage );
			kv->SetPtr( "contextlabel", m_pContextLabel );
			PostActionSignal( kv );
			return;
		}
		else if ( !Q_stricmp( cmd, "close_tab" ) )
		{
			TryPageClose();
			return;
		}

		BaseClass::OnCommand( cmd );		
	}

	void TryPageClose()
	{
		if ( !m_pParent->AllowClosing() )
			return;

		int ipagenum = m_pParent->FindPage( m_pPage );
		KeyValues *pKV = new KeyValues( "AskPageClose" );
		pKV->SetPtr( "PageTab", this );
		pKV->SetInt( "TabIndex", ipagenum );
		m_pParent->PostActionSignal( pKV );
	}

	MESSAGE_FUNC( OnCloseConfirmed, "OnCloseConfirmed" )
	{
		m_pParent->ClosePage( m_pPage );
	}

	void RemovePageAndTab()
	{
		if ( !m_bShouldShowCloseButton || !m_pButtonClose )
			return;

		if ( m_pParent->GetNumPages() > 1 )
		{
			int ipagenum = m_pParent->FindPage( m_pPage );
			bool bWasActive = m_pPage == m_pParent->GetActivePage();

			m_pParent->DeletePage( m_pPage );
			m_pParent->PostActionSignal(new KeyValues("PageClosed", "pagenum", ipagenum));

			if ( bWasActive )
				m_pParent->ScrollToActivePage();
		}
	}

	IBorder *GetBorder(bool depressed, bool armed, bool selected, bool keyfocus)
	{
		if (_active)
		{
			return m_pActiveBorder;
		}
		return m_pNormalBorder;
	}

	virtual Color GetButtonFgColor()
	{
		if (_active)
		{
			return _textColor;
		}
		else
		{
			return _dimTextColor;
		}
	}

	virtual void SetActive(bool state)
	{
		_active = state;
		SetZPos( state ? 100 : 0 );
		InvalidateLayout();
		Repaint();
	}

	virtual void SetTabWidth( int iWidth )
	{
		m_bMaxTabWidth = iWidth;
		InvalidateLayout();
	}

	virtual bool CanBeDefaultButton(void)
	{
		return false;
	}

	virtual void OnCursorMoved( int x, int y )
	{
		if ( m_bDragEnabled && input()->IsMouseDown( MOUSE_LEFT ) )
		{
			if ( input()->GetMouseCapture() != GetVPanel() )
				input()->SetMouseCapture(GetVPanel());

			if ( m_iLastMouseX < 0 || abs( m_iLastMouseX - x ) > 3 )
			{
				m_iLastMouseX = -1;
				OnDoDrag();
			}
		}
		else
			m_iLastMouseX = x;

		BaseClass::OnCursorMoved( x, y );
	}

	virtual void OnDoDrag()
	{
		int mx, my;
		input()->GetCursorPosition( mx, my );



		//LocalToScreen( mx, my );
		m_pParent->ScreenToLocal( mx, my );

		int iBestTab = -1;
		int iBestDist = 99999;
		for ( int i = 0; i < m_pParent->GetNumPages() - 1; i++ )
		{
			PageTab *pTab0 = m_pParent->GetTab(i);
			PageTab *pTab1 = m_pParent->GetTab(i+1);
			int _x0, _x1, dummy;
			pTab0->GetBounds( _x0, dummy, _x1, dummy );
			_x1 += _x0;
			int xmin = _x0;
			pTab1->GetBounds( _x0, dummy, _x1, dummy );
			_x1 += _x0;
			int xmax = _x1;

			if ( mx < xmin || mx > xmax )
				continue;

			int mid = (xmax-xmin)*0.5f+xmin;
			int dist = abs( mx - mid );

			if ( dist > iBestDist )
				continue;

			iBestDist = dist;
			iBestTab = ( mx >= mid ) ? i + 1 : i;
		}

		if ( iBestTab >= 0 )
			m_pParent->MoveTab( this, iBestTab );
	}

	//Fire action signal when mouse is pressed down instead  of on release.
	virtual void OnMousePressed(MouseCode code) 
	{
		// check for context menu open
		if (!IsEnabled())
			return;

		if ( code == MOUSE_MIDDLE )
		{
			TryPageClose();
			return;
		}
		
		if (!IsMouseClickEnabled(code))
			return;

		if (IsUseCaptureMouseEnabled())
		{
			{
				RequestFocus();
				FireActionSignal();
				SetSelected(true);
				Repaint();
			}
			
			// lock mouse input to going to this button
			input()->SetMouseCapture(GetVPanel());
		}
	}

	virtual void OnMouseReleased(MouseCode code)
	{
		if (input()->GetMouseCapture() == GetVPanel())
		// ensure mouse capture gets released
		//if (IsUseCaptureMouseEnabled())
		{
			input()->SetMouseCapture(NULL);
		}

		// make sure the button gets unselected
		SetSelected(false);
		Repaint();

		if (code == MOUSE_RIGHT)
		{
			KeyValues *kv = new KeyValues("OpenContextMenu");
			kv->SetPtr( "page", m_pPage );
			kv->SetPtr( "contextlabel", m_pContextLabel );
			PostActionSignal( kv );
		}
	}

	void OnMouseWheeled( int d )
	{
		m_pParent->DoScroll( d * -100 );
	}
};

class CTabBar : public Panel
{
	DECLARE_CLASS_SIMPLE( CTabBar, Panel );
public:

	CTabBar( PropertySheet *parent ) : BaseClass( parent ) {
		m_pParent = parent;
	};

	void OnMouseDoublePressed( MouseCode code )
	{
		GetParent()->OnCommand( "add_tab" );
		BaseClass::OnMouseDoublePressed( code );
	}

	void OnMouseWheeled( int d )
	{
		m_pParent->DoScroll( d * -100 );
	}

	PropertySheet *m_pParent;
};

}; // namespace vgui

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
PropertySheet::PropertySheet(
	Panel *parent, 
	const char *panelName, 
	bool draggableTabs /*= false*/,
	bool closeableTabs ) : BaseClass(parent, panelName)
{
	_activePage = NULL;
	_activeTab = NULL;
	_tabWidth = 64;
	_activeTabIndex = 0;
	_showTabs = true;
	_combo = NULL;
    _tabFocus = false;
	m_flPageTransitionEffectTime = 0.0f;
	m_bSmallTabs = false;
	m_tabFont = 0;
	m_bDraggableTabs = draggableTabs;
	m_pTabKV = NULL;
	m_bCloseableTabs = closeableTabs;

	//if ( m_bDraggableTabs )
	//{
	//	SetDropEnabled( true );
	//}

	m_bKBNavigationEnabled = true;

	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor, associates pages with a combo box
//-----------------------------------------------------------------------------
PropertySheet::PropertySheet(Panel *parent, const char *panelName, ComboBox *combo) : BaseClass(parent, panelName)
{
	_activePage = NULL;
	_activeTab = NULL;
	_tabWidth = 64;
	_activeTabIndex = 0;
	_combo=combo;
	_combo->AddActionSignalTarget(this);
	_showTabs = false;
    _tabFocus = false;
	m_flPageTransitionEffectTime = 0.0f;
	m_bSmallTabs = false;
	m_tabFont = 0;
	m_bDraggableTabs = false;
	m_pTabKV = NULL;
	m_bCloseableTabs = false;

	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
PropertySheet::~PropertySheet()
{
}

void PropertySheet::Init()
{
	m_iTabHeight = 28;

	m_iTabXScroll_Target = 0;
	m_flTabXScroll_Accum = 0;
	m_iTabXScroll_Cur = 0;
	m_dLastTime = 0;
	m_dFrametime = 0;

	m_flMouseScroll = 0;

	m_pBut_Left = new Button( this, "scroll_left", "<", this, "scroll_left" );
	m_pBut_Right = new Button( this, "scroll_right", ">", this, "scroll_right" );

	m_pBut_Left->SetTextInset( 6, -1 );
	m_pBut_Right->SetTextInset( 6, -1 );

	//m_pBut_Left->MakePopup( false );
	//m_pBut_Right->MakePopup( false );

	m_pBut_Left->SetVisible( false );
	m_pBut_Right->SetVisible( false );

	pTabBar = new CTabBar( this );
	m_pAddTab = NULL;
}

void PropertySheet::SetAddTabButtonEnabled( bool bEnabled )
{
	if ( !!m_pAddTab == bEnabled )
		return;

	if ( bEnabled )
	{
		m_pAddTab = new Button( pTabBar, "add_tab", "+", this, "add_tab" );
		InvalidateLayout();
	}
	else
	{
		m_pAddTab->MarkForDeletion();
		m_pAddTab = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: ToolWindow uses this to drag tools from container to container by dragging the tab
// Input  :  - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool PropertySheet::IsDraggableTab() const
{
	return m_bDraggableTabs;
}

void PropertySheet::SetDraggableTabs( bool state )
{
	m_bDraggableTabs = state;
}

//-----------------------------------------------------------------------------
// Purpose: Lower profile tabs
// Input  : state - 
//-----------------------------------------------------------------------------
void PropertySheet::SetSmallTabs( bool state )
{
	m_bSmallTabs = state;
	m_tabFont = scheme()->GetIScheme( GetScheme() )->GetFont( m_bSmallTabs ? "DefaultVerySmall" : "Default" );
	int c = m_PageTabs.Count();
	for ( int i = 0; i < c ; ++i )
	{
		PageTab *tab = m_PageTabs[ i ];
		Assert( tab );
		tab->SetFont( m_tabFont );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool PropertySheet::IsSmallTabs() const
{
	return m_bSmallTabs;
}


int PropertySheet::GetTabHeight()
{
	return m_iTabHeight;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
void PropertySheet::ShowContextButtons( bool state )
{
	m_bContextButton = state;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool PropertySheet::ShouldShowContextButtons() const
{
	return m_bContextButton;
}

int PropertySheet::FindPage( Panel *page ) const
{
	int c = m_Pages.Count();
	for ( int i = 0; i < c; ++i )
	{
		if ( m_Pages[ i ].page == page )
			return i;
	}

	return m_Pages.InvalidIndex();
}

//-----------------------------------------------------------------------------
// Purpose: adds a page to the sheet
//-----------------------------------------------------------------------------
void PropertySheet::AddPage(Panel *page, const char *title, char const *imageName /*= NULL*/, bool bHasContextMenu /*= false*/ )
{
	if (!page)
		return;

	// don't add the page if we already have it
	if ( FindPage( page ) != m_Pages.InvalidIndex() )
		return;

	long hoverActivatePageTime = 250;
	PageTab *tab = new PageTab(this, "tab", title, imageName, _tabWidth, page, m_bContextButton && bHasContextMenu, hoverActivatePageTime, m_bCloseableTabs );
	tab->SetParent( pTabBar );

	//if ( m_bDraggableTabs )
	//{
	//	tab->SetDragEnabled( true );
	//}

	tab->SetFont( m_tabFont );
	if(_showTabs)
	{
		tab->AddActionSignalTarget(this);
	}
	else if (_combo)
	{
		_combo->AddItem(title, NULL);
	}

	if ( m_pTabKV )
	{
		tab->ApplySettings( m_pTabKV );
	}

	m_PageTabs.AddToTail(tab);

	Page_t info;
	info.page = page;
	info.contextMenu = m_bContextButton && bHasContextMenu;
	
	m_Pages.AddToTail( info );

	page->SetParent(this);
	page->AddActionSignalTarget(this);
	PostMessage(page, new KeyValues("ResetData"));

	page->SetVisible(false);
	InvalidateLayout();

	if (!_activePage)
	{
		// first page becomes the active page
		ChangeActiveTab( 0 );
		if ( _activePage )
		{
			_activePage->RequestFocus( 0 );
		}
	}

	UpdateTabCloseButtons();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::SetActivePage(Panel *page)
{
	// walk the list looking for this page
	int index = FindPage( page );
	if (!m_Pages.IsValidIndex(index))
		return;

	ChangeActiveTab(index);

	ScrollToActivePage();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::SetTabWidth(int pixels)
{
	_tabWidth = pixels;
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: reloads the data in all the property page
//-----------------------------------------------------------------------------
void PropertySheet::ResetAllData()
{
	// iterate all the dialogs resetting them
	for (int i = 0; i < m_Pages.Count(); i++)
	{
		ipanel()->SendMessage(m_Pages[i].page->GetVPanel(), new KeyValues("ResetData"), GetVPanel());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Applies any changes made by the dialog
//-----------------------------------------------------------------------------
void PropertySheet::ApplyChanges()
{
	// iterate all the dialogs resetting them
	for (int i = 0; i < m_Pages.Count(); i++)
	{
		ipanel()->SendMessage(m_Pages[i].page->GetVPanel(), new KeyValues("ApplyChanges"), GetVPanel());
	}
}

//-----------------------------------------------------------------------------
// Purpose: gets a pointer to the currently active page
//-----------------------------------------------------------------------------
Panel *PropertySheet::GetActivePage()
{
	return _activePage;
}

//-----------------------------------------------------------------------------
// Purpose: gets a pointer to the currently active tab
//-----------------------------------------------------------------------------
Panel *PropertySheet::GetActiveTab()
{
	return _activeTab;
}

void PropertySheet::MoveTab( PageTab *pTab, int newIndex )
{
	int oldIndex = m_PageTabs.Find( pTab );

	if ( !m_PageTabs.IsValidIndex( oldIndex ) )
		return;

	if ( newIndex == oldIndex )
		return;

	bool bAfter = oldIndex < newIndex;
	if ( bAfter )
		newIndex--;

	Page_t tPage = m_Pages[oldIndex];

	m_PageTabs.Remove( oldIndex );
	m_Pages.Remove( oldIndex );
	Assert( m_PageTabs.IsValidIndex( newIndex ) );

	if ( bAfter )
	{
		m_PageTabs.InsertAfter( newIndex, pTab );
		m_Pages.InsertAfter( newIndex, tPage );
	}
	else
	{
		m_PageTabs.InsertBefore( newIndex, pTab );
		m_Pages.InsertBefore( newIndex, tPage );
	}

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: returns the number of panels in the sheet
//-----------------------------------------------------------------------------
int PropertySheet::GetNumPages()
{
	return m_Pages.Count();
}

//-----------------------------------------------------------------------------
// Purpose: returns the name contained in the active tab
// Input  : a text buffer to contain the output 
//-----------------------------------------------------------------------------
void PropertySheet::GetActiveTabTitle(char *textOut, int bufferLen)
{
	if(_activeTab) _activeTab->GetText(textOut, bufferLen);
}

//-----------------------------------------------------------------------------
// Purpose: returns the name contained in the active tab
// Input  : a text buffer to contain the output 
//-----------------------------------------------------------------------------
bool PropertySheet::GetTabTitle(int i, char *textOut, int bufferLen)
{
	if (i < 0 && i > m_PageTabs.Count()) 
	{
		return false;
	}

	m_PageTabs[i]->GetText(textOut, bufferLen);
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the index of the currently active page
//-----------------------------------------------------------------------------
int PropertySheet::GetActivePageNum()
{
	for (int i = 0; i < m_Pages.Count(); i++)
	{
		if (m_Pages[i].page == _activePage) 
		{
			return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Forwards focus requests to current active page
//-----------------------------------------------------------------------------
void PropertySheet::RequestFocus(int direction)
{
    if (direction == -1 || direction == 0)
    {
    	if (_activePage)
    	{
    		_activePage->RequestFocus(direction);
            _tabFocus = false;
    	}
    }
    else 
    {
        if (_showTabs && _activeTab)
        {
            _activeTab->RequestFocus(direction);
            _tabFocus = true;
        }
		else if (_activePage)
    	{
    		_activePage->RequestFocus(direction);
            _tabFocus = false;
    	}
    }
}

//-----------------------------------------------------------------------------
// Purpose: moves focus back
//-----------------------------------------------------------------------------
bool PropertySheet::RequestFocusPrev(VPANEL panel)
{
    if (_tabFocus || !_showTabs || !_activeTab)
    {
        _tabFocus = false;
        return BaseClass::RequestFocusPrev(panel);
    }
    else
    {
        if (GetVParent())
        {
            PostMessage(GetVParent(), new KeyValues("FindDefaultButton"));
        }
        _activeTab->RequestFocus(-1);
        _tabFocus = true;
        return true;
    }
}

//-----------------------------------------------------------------------------
// Purpose: moves focus forward
//-----------------------------------------------------------------------------
bool PropertySheet::RequestFocusNext(VPANEL panel)
{
    if (!_tabFocus || !_activePage)
    {
        return BaseClass::RequestFocusNext(panel);
    }
    else
    {
        if (!_activeTab)
        {
            return BaseClass::RequestFocusNext(panel);
        }
        else
        {
            _activePage->RequestFocus(1);
            _tabFocus = false;
            return true;
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Gets scheme settings
//-----------------------------------------------------------------------------
void PropertySheet::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// a little backwards-compatibility with old scheme files
	IBorder *pBorder = pScheme->GetBorder("PropertySheetBorder");
	if (pBorder == pScheme->GetBorder("Default"))
	{
		// get the old name
		pBorder = pScheme->GetBorder("RaisedBorder");
	}

	SetBorder(pBorder);
	m_flPageTransitionEffectTime = atof(pScheme->GetResourceString("PropertySheet.TransitionEffectTime"));

	m_tabFont = pScheme->GetFont( m_bSmallTabs ? "DefaultVerySmall" : "Default" );

	if ( m_pTabKV )
	{
		for (int i = 0; i < m_PageTabs.Count(); i++)
		{
			m_PageTabs[i]->ApplySettings( m_pTabKV );
		}
	}

	/*
	if ( !IsProportional() )
	{
		m_iTabHeight = scheme()->GetProportionalNormalizedValueEx( GetScheme(), m_iTabHeight );
		m_iTabHeightSmall = scheme()->GetProportionalNormalizedValueEx( GetScheme(), m_iTabHeightSmall );
	}
	*/
	m_iTabHeight = 28;
	m_iTabHeightSmall = 14;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	KeyValues *pTabKV = inResourceData->FindKey( "tabskv" );
	if ( pTabKV )
	{
		if ( m_pTabKV )
		{
			m_pTabKV->deleteThis();
		}
		m_pTabKV = new KeyValues("tabkv");
		pTabKV->CopySubkeys( m_pTabKV );
	}

	KeyValues *pTabWidthKV = inResourceData->FindKey( "tabwidth" );
	if ( pTabWidthKV )
	{
		_tabWidth = scheme()->GetProportionalScaledValueEx(GetScheme(), pTabWidthKV->GetInt());
		for (int i = 0; i < m_PageTabs.Count(); i++)
		{
			m_PageTabs[i]->SetTabWidth( _tabWidth );
		}
	}

	KeyValues *pTransitionKV = inResourceData->FindKey( "transition_time" );
	if ( pTransitionKV )
	{
		m_flPageTransitionEffectTime = pTransitionKV->GetFloat();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Paint our border specially, with the tabs in mind
//-----------------------------------------------------------------------------
void PropertySheet::PaintBorder()
{
	IBorder *border = GetBorder();
	if (!border)
		return;

	// draw the border underneath the buttons, with a break
	int wide, tall;
	GetSize(wide, tall);

	// draw the border, but with a break at the active tab
	int px = 0, py = 0, pwide = 0, ptall = 0;
	if (_activeTab)
	{
		bool bShowArrows = ShouldShowArrows();

		_activeTab->GetBounds(px, py, pwide, ptall);
		ptall -= 1;

		if ( bShowArrows )
			px += GetScrollButtonSize();
	}

	border->Paint(0, py + ptall, wide, tall, IBorder::SIDE_TOP, px + 1, px + pwide - 1);
}

bool PropertySheet::ShouldShowArrows()
{
	if ( !_showTabs )
		return false;

	int sx, sy;
	GetSize( sx, sy );

	return GetAccumTabX() > sx;
}

int PropertySheet::GetAccumTabX()
{
	int xtab;
	xtab = m_iTabXIndent * 2;

	int width, tall;
	for (int i = 0; i < m_PageTabs.Count(); i++)
	{
		//m_PageTabs[i]->GetSize(width, tall);
		width = m_PageTabs[i]->GetDesignatedWidth();
		xtab += (width + 1) + m_iTabXDelta;
	}

	if ( m_pAddTab )
	{
		m_pAddTab->GetSize( width, tall );
		xtab += width + 1;
	}

	return xtab;
}

//-----------------------------------------------------------------------------
// Purpose: Lays out the dialog
//-----------------------------------------------------------------------------
void PropertySheet::PerformLayout()
{
	BaseClass::PerformLayout();

	int x, y, wide, tall;
	GetBounds(x, y, wide, tall);
	if (_activePage)
	{
		int tabHeight = IsSmallTabs() ? m_iTabHeightSmall : m_iTabHeight;

		if(_showTabs)
		{
			_activePage->SetBounds(0, tabHeight, wide, tall - tabHeight);
		}
		else
		{
			_activePage->SetBounds(0, 0, wide, tall );
		}
		_activePage->InvalidateLayout();
	}

	int tabHeight = IsSmallTabs() ? (m_iTabHeightSmall-1) : (m_iTabHeight-1);

	bool bShowArrows = ShouldShowArrows();
	//int tabxSize = GetAccumTabX();
	const int buttonscrollsize = GetScrollButtonSize();

	if ( !bShowArrows )
	{
		m_iTabXScroll_Cur = 0;
		pTabBar->SetSize( wide, tabHeight + 1 );
		pTabBar->SetPos( 0, 0 );
	}
	else
	{
		//m_iTabXScroll = clamp( m_iTabXScroll, 0, tabxSize - wide + buttonscrollsize * 2 );
		pTabBar->SetSize( wide - buttonscrollsize * 2, tabHeight + 1 );
		pTabBar->SetPos( buttonscrollsize, 0 );
	}
	
	int xtab;
	int limit = m_PageTabs.Count();

	xtab = m_iTabXIndent - m_iTabXScroll_Cur;

	// draw the visible tabs
	if (_showTabs)
	{
		for (int i = 0; i < limit; i++)
		{

            int width, tall;
            m_PageTabs[i]->GetSize(width, tall);
			if (m_PageTabs[i] == _activeTab)
			{
				// active tab is taller
				_activeTab->SetBounds(xtab, 2, width, tabHeight);
			}
			else
			{
				m_PageTabs[i]->SetBounds(xtab, 4, width, tabHeight - 2);
			}
			m_PageTabs[i]->SetVisible(true);
			xtab += (width + 1) + m_iTabXDelta;
		}

		if ( m_pAddTab )
		{
			m_pAddTab->SetVisible( true );
			m_pAddTab->SetBounds( xtab, 2, buttonscrollsize, tabHeight );
			xtab += buttonscrollsize + 1;
		}
	}
	else
	{
		for (int i = 0; i < limit; i++)
		{
			m_PageTabs[i]->SetVisible(false);
		}

		if ( m_pAddTab )
			m_pAddTab->SetVisible( false );
	}

	// ensure draw order (page drawing over all the tabs except one)
	if (_activePage)
	{
		_activePage->MoveToFront();
		_activePage->Repaint();
	}
	if (_activeTab)
	{
		_activeTab->MoveToFront();
		_activeTab->Repaint();
	}

	m_pBut_Left->SetPos( 0, 2 );
	m_pBut_Right->SetPos( wide - GetScrollButtonSize(), 2 );

	m_pBut_Left->SetSize( buttonscrollsize, tabHeight );
	m_pBut_Right->SetSize( buttonscrollsize, tabHeight );

	m_pBut_Left->SetVisible( bShowArrows );
	m_pBut_Right->SetVisible( bShowArrows );

	Color fgCol = Color( 255,255,255,255 );
	Color bgCol = Color( 127,127,127,255 );
	Color armedfg = Color( 0, 0, 0, 255 );
	Color armedbg = Color( 255,255,255,255 );

	m_pBut_Left->SetDefaultColor( fgCol, bgCol );
	m_pBut_Right->SetDefaultColor( fgCol, bgCol );
	m_pBut_Left->SetDepressedColor( armedfg, armedbg );
	m_pBut_Right->SetDepressedColor( armedfg, armedbg );

	if ( m_pAddTab )
	{
		m_pAddTab->SetDefaultColor( fgCol, bgCol );
		m_pAddTab->SetDepressedColor( armedfg, armedbg );
	}
}

void PropertySheet::OnThink()
{
	double curTime = system()->GetCurrentTime();
	m_dFrametime = curTime - m_dLastTime;
	m_dLastTime = curTime;

	if ( ShouldShowArrows() )
	{
		int wide, tall;
		GetSize( wide, tall );
		m_iTabXScroll_Target = clamp( m_iTabXScroll_Target, 0, GetAccumTabX() - wide + GetScrollButtonSize() * 2 );

		int iOldPos = m_iTabXScroll_Cur;

		if ( abs( m_iTabXScroll_Target - m_flTabXScroll_Accum ) > 1.0f )
			m_flTabXScroll_Accum += ( (float)m_iTabXScroll_Target - m_flTabXScroll_Accum ) * min( 1.0f, m_dFrametime * 16.0f );
			//m_flTabXScroll_Accum = Approach( m_iTabXScroll_Target, m_flTabXScroll_Accum, m_dFrametime * 1000.0f );
		else
			m_flTabXScroll_Accum = m_iTabXScroll_Target;

		m_iTabXScroll_Cur = m_flTabXScroll_Accum;

		if ( iOldPos != m_iTabXScroll_Cur )
			InvalidateLayout();
	}
	else
	{
		m_flTabXScroll_Accum = 0;
		m_iTabXScroll_Cur = 0;
		m_iTabXScroll_Target = 0;
	}

	for ( int i = 0; i < m_PageTabs.Count(); i++ )
	{
		if ( m_PageTabs[i]->IsAnimating() )
		{
			m_PageTabs[i]->InvalidateLayout();
			InvalidateLayout();
		}
	}

	m_pBut_Left->MoveToFront();
	m_pBut_Right->MoveToFront();

	BaseClass::OnThink();
}

void PropertySheet::ClosePage( Panel *page )
{
	int i = FindPage( page );
	if ( i == m_PageTabs.InvalidIndex() )
		return;

	m_PageTabs[i]->CloseTab();
}

bool PropertySheet::DoScroll()
{
	if ( !ShouldShowArrows() )
		return false;

	int x, y, sx, sy;
	input()->GetCursorPosition( x, y );
	ScreenToLocal( x, y );
	GetSize( sx, sy );

	int min = GetScrollButtonSize();
	int max = sx - GetScrollButtonSize();

	if ( x > min && x < max )
	{
		m_flMouseScroll = 0;
		return false;
	}

	if ( x > min )
		x-=max-1;
	else
		x -= min;

	m_flMouseScroll += ( system()->GetCurrentTime() - system()->GetFrameTime() ) * x * 1000.0l;

	if ( abs(m_flMouseScroll) < 1.0l )
		return false;

	double integral = floor( m_flMouseScroll );
	m_iTabXScroll_Target += integral;
	m_flMouseScroll -= integral;
	InvalidateLayout();
	return true;
}

void PropertySheet::DoScroll( int delta )
{
	if ( !ShouldShowArrows() )
		return;

	m_iTabXScroll_Target += delta;
	InvalidateLayout();
}

void PropertySheet::ScrollToActivePage()
{
	if ( !ShouldShowArrows() )
		return;

	PageTab *pActive = (PageTab*)GetActiveTab();

	if ( !pActive )
		return;

	InvalidateLayout(true);
	pActive->InvalidateLayout(true);

	int tab_px, tab_py, tab_sx, tab_sy;
	pActive->GetBounds( tab_px, tab_py, tab_sx, tab_sy );
	tab_sx = pActive->GetDesignatedWidth();
	int sheet_px, sheet_py, sheet_sx, sheet_sy;
	GetBounds( sheet_px, sheet_py, sheet_sx, sheet_sy );

	//page_px += m_iTabXScroll;
	int containersize = sheet_sx - GetScrollButtonSize() * 2;
	int extent_x = tab_px + tab_sx;

	if ( m_pAddTab && GetActiveTab() == GetTab( GetNumPages() - 1 ) )
	{
		extent_x += 1 + GetScrollButtonSize();
	}

	if ( tab_px < GetScrollButtonSize() )
		m_iTabXScroll_Target += tab_px; //+= GetScrollButtonSize() - tab_px;
	else if ( extent_x > containersize )
		m_iTabXScroll_Target += extent_x - containersize + 1;
	else
		return;

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Switches the active panel
//-----------------------------------------------------------------------------
void PropertySheet::OnTabPressed(Panel *panel)
{
	// look for the tab in the list
	for (int i = 0; i < m_PageTabs.Count(); i++)
	{
		if (m_PageTabs[i] == panel)
		{
			// flip to the new tab
			ChangeActiveTab(i);
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns the panel associated with index i
// Input  : the index of the panel to return 
//-----------------------------------------------------------------------------
Panel *PropertySheet::GetPage(int i) 
{
	if(i<0 && i>m_Pages.Count()) 
	{
		return NULL;
	}

	return m_Pages[i].page;
}

PageTab *PropertySheet::GetTab(int i)
{
	if ( !m_PageTabs.IsValidIndex(i) )
		return NULL;

	return m_PageTabs[i];
}

void PropertySheet::SetTabColor( int index, Color c )
{
	if ( !m_PageTabs.IsValidIndex(index) )
		return;

	m_PageTabs[index]->SetCustomBgColor( c );
}

//-----------------------------------------------------------------------------
// Purpose: disables page by name
//-----------------------------------------------------------------------------
void PropertySheet::DisablePage(const char *title) 
{
	SetPageEnabled(title, false);
}

//-----------------------------------------------------------------------------
// Purpose: enables page by name
//-----------------------------------------------------------------------------
void PropertySheet::EnablePage(const char *title) 
{
	SetPageEnabled(title, true);
}

//-----------------------------------------------------------------------------
// Purpose: enabled or disables page by name
//-----------------------------------------------------------------------------
void PropertySheet::SetPageEnabled(const char *title, bool state) 
{
	for (int i = 0; i < m_PageTabs.Count(); i++)
	{
		if (_showTabs)
		{
			char tmp[50];
			m_PageTabs[i]->GetText(tmp,50);
			if (!strnicmp(title,tmp,strlen(tmp)))
			{	
				m_PageTabs[i]->SetEnabled(state);
			}
		}
		else
		{
			_combo->SetItemEnabled(title,state);
		}
	}
}

void PropertySheet::RemoveAllPages()
{
	int c = m_Pages.Count();
	for ( int i = c - 1; i >= 0 ; --i )
	{
		RemovePage( m_Pages[ i ].page );
	}
}

void PropertySheet::UpdateTabCloseButtons()
{
	bool bShowCloseButtons = AllowClosing();

	for ( int i = 0; i < m_PageTabs.Count(); i++ )
	{
		m_PageTabs[i]->SetCloseButtonVisible( bShowCloseButtons );
		m_PageTabs[i]->InvalidateLayout(true);
	}

	InvalidateLayout( false, true );
}

int PropertySheet::GetNumActiveTabs()
{
	int activePages = 0;
	for ( int i = 0; i < m_PageTabs.Count(); i++ )
		if ( !m_PageTabs[i]->IsClosing() )
			activePages++;
	return activePages;
}

bool PropertySheet::AllowClosing()
{
	return GetNumActiveTabs() > 1;
}

//-----------------------------------------------------------------------------
// Purpose: deletes the page associated with panel
// Input  : *panel - the panel of the page to remove
//-----------------------------------------------------------------------------
void PropertySheet::RemovePage(Panel *panel) 
{
	int location = FindPage( panel );
	if ( location == m_Pages.InvalidIndex() )
		return;

	// Since it's being deleted, don't animate!!!
	m_hPreviouslyActivePage = NULL;

	// ASSUMPTION = that the number of pages equals number of tabs
	if( _showTabs )
	{
		m_PageTabs[location]->RemoveActionSignalTarget( this );
	}
	// now remove the tab
	PageTab *tab  = m_PageTabs[ location ];
	m_PageTabs.Remove( location );
	tab->MarkForDeletion();
	
	// Remove from page list
	m_Pages.Remove( location );

	// Unparent
	panel->SetParent( (Panel *)NULL );

	if ( _activePage == panel ) 
	{
		_activeTab = NULL;
		_activePage = NULL;
		// if this page is currently active, backup to the page before this.
		if ( GetNumPages() <= location )
			location--;
		ChangeActiveTab( max( location, 0 ) ); 
	}

	PerformLayout();

	UpdateTabCloseButtons();
}

//-----------------------------------------------------------------------------
// Purpose: deletes the page associated with panel
// Input  : *panel - the panel of the page to remove
//-----------------------------------------------------------------------------
void PropertySheet::DeletePage(Panel *panel) 
{
	Assert( panel );
	RemovePage( panel );
	panel->MarkForDeletion();
}

//-----------------------------------------------------------------------------
// Purpose: flips to the new tab, sending out all the right notifications
//			flipping to a tab activates the tab.
//-----------------------------------------------------------------------------
void PropertySheet::ChangeActiveTab( int index )
{
	if ( !m_Pages.IsValidIndex( index ) )
	{
		_activeTab = NULL;
		if ( m_Pages.Count() > 0 )
		{
			_activePage = NULL;
			ChangeActiveTab( 0 );
		}
		return;
	}

	if ( m_Pages[index].page == _activePage )
	{
		if ( _activeTab )
		{
			_activeTab->RequestFocus();
		}
		_tabFocus = true;
		return;
	}

	int c = m_Pages.Count();
	for ( int i = 0; i < c; ++i )
	{
		m_Pages[ i ].page->SetVisible( false );
	}

	m_hPreviouslyActivePage = _activePage;
	// notify old page
	if (_activePage)
	{
		ivgui()->PostMessage(_activePage->GetVPanel(), new KeyValues("PageHide"), GetVPanel());
		KeyValues *msg = new KeyValues("PageTabActivated");
		msg->SetPtr("panel", (Panel *)NULL);
		ivgui()->PostMessage(_activePage->GetVPanel(), msg, GetVPanel());
	}
	if (_activeTab)
	{
		//_activeTabIndex=index;
		_activeTab->SetActive(false);

		// does the old tab have the focus?
		_tabFocus = _activeTab->HasFocus();
	}
	else
	{
		_tabFocus = false;
	}

	// flip page
	_activePage = m_Pages[index].page;
	_activeTab = m_PageTabs[index];
	_activeTabIndex = index;

	_activePage->SetVisible(true);
	_activePage->MoveToFront();
	
	_activeTab->SetVisible(true);
	_activeTab->MoveToFront();
	_activeTab->SetActive(true);

	if (_tabFocus)
	{
		// if a tab already has focused,give the new tab the focus
		_activeTab->RequestFocus();
	}
	else
	{
		// otherwise, give the focus to the page
		_activePage->RequestFocus();
	}

	if (!_showTabs)
	{
		_combo->ActivateItemByRow(index);
	}

	_activePage->MakeReadyForUse();

	// transition effect
	if (m_flPageTransitionEffectTime)
	{
		if (m_hPreviouslyActivePage.Get())
		{
			// fade out the previous page
			GetAnimationController()->RunAnimationCommand(m_hPreviouslyActivePage, "Alpha", 0.0f, 0.0f, m_flPageTransitionEffectTime / 2, AnimationController::INTERPOLATOR_LINEAR);
		}

		// fade in the new page
		_activePage->SetAlpha(0);
		GetAnimationController()->RunAnimationCommand(_activePage, "Alpha", 255.0f, m_flPageTransitionEffectTime / 2, m_flPageTransitionEffectTime / 2, AnimationController::INTERPOLATOR_LINEAR);
	}
	else
	{
		if (m_hPreviouslyActivePage.Get())
		{
			// no transition, just hide the previous page
			m_hPreviouslyActivePage->SetVisible(false);
		}
		_activePage->SetAlpha( 255 );
	}

	// notify
	ivgui()->PostMessage(_activePage->GetVPanel(), new KeyValues("PageShow"), GetVPanel());

	KeyValues *msg = new KeyValues("PageTabActivated");
	msg->SetPtr("panel", (Panel *)_activeTab);
	ivgui()->PostMessage(_activePage->GetVPanel(), msg, GetVPanel());

	// tell parent
	PostActionSignal(new KeyValues("PageChanged"));

	// Repaint
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Gets the panel with the specified hotkey, from the current page
//-----------------------------------------------------------------------------
Panel *PropertySheet::HasHotkey(wchar_t key)
{
	if (!_activePage)
		return NULL;

	for (int i = 0; i < _activePage->GetChildCount(); i++)
	{
		Panel *hot = _activePage->GetChild(i)->HasHotkey(key);
		if (hot)
		{
			return hot;
		}
	}
	
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: catches the opencontextmenu event
//-----------------------------------------------------------------------------
void PropertySheet::OnOpenContextMenu( KeyValues *params )
{
	// tell parent
	KeyValues *kv = params->MakeCopy();
	PostActionSignal( kv );
	Panel *page = reinterpret_cast< Panel * >( params->GetPtr( "page" ) );
	if ( page )
	{
		PostMessage( page->GetVPanel(), params->MakeCopy() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle key presses, through tabs.
//-----------------------------------------------------------------------------
void PropertySheet::OnKeyCodeTyped(KeyCode code)
{
	bool shift = (input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT));
	bool ctrl = (input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL));
	bool alt = (input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT));
	
	if ( ctrl && shift && alt && code == KEY_B)
	{
		// enable build mode
		EditablePanel *ep = dynamic_cast< EditablePanel * >( GetActivePage() );
		if ( ep )
		{
			ep->ActivateBuildMode();
			return;
		}
	}

	if ( IsKBNavigationEnabled() )
	{
		switch (code)
		{
			// for now left and right arrows just open or close submenus if they are there.
		case KEY_RIGHT:
			{
				ChangeActiveTab(_activeTabIndex+1);
				break;
			}
		case KEY_LEFT:
			{
				ChangeActiveTab(_activeTabIndex-1);
				break;
			}
		default:
			BaseClass::OnKeyCodeTyped(code);
			break;
		}
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called by the associated combo box (if in that mode), changes the current panel
//-----------------------------------------------------------------------------
void PropertySheet::OnTextChanged(Panel *panel,const wchar_t *wszText)
{
	if ( panel == _combo )
	{
		wchar_t tabText[30];
		for(int i = 0 ; i < m_PageTabs.Count() ; i++ )
		{
			tabText[0] = 0;
			m_PageTabs[i]->GetText(tabText,30);
			if ( !wcsicmp(wszText,tabText) )
			{
				ChangeActiveTab(i);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::OnCommand(const char *command)
{
    // propogate the close command to our parent
	if (!stricmp(command, "Close") && GetVParent())
    {
		CallParentFunction(new KeyValues("Command", "command", command));
    }
	else if (!stricmp(command, "scroll_left"))
	{
		m_iTabXScroll_Target -= 100;
		InvalidateLayout();
	}
	else if (!stricmp(command, "scroll_right"))
	{
		m_iTabXScroll_Target += 100;
		InvalidateLayout();
	}
	else if ( !stricmp(command, "add_tab" ) )
	{
		PostActionSignal( new KeyValues( "RequestAddTab" ) );
	}
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::OnApplyButtonEnable()
{
	// tell parent
	PostActionSignal(new KeyValues("ApplyButtonEnable"));	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::OnCurrentDefaultButtonSet( vgui::VPANEL defaultButton )
{
	// forward the message up
	if (GetVParent())
	{
		KeyValues *msg = new KeyValues("CurrentDefaultButtonSet");
		msg->SetInt("button", ivgui()->PanelToHandle( defaultButton ) );
		PostMessage(GetVParent(), msg);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::OnDefaultButtonSet( VPANEL defaultButton )
{
	// forward the message up
	if (GetVParent())
	{
		KeyValues *msg = new KeyValues("DefaultButtonSet");
		msg->SetInt("button", ivgui()->PanelToHandle( defaultButton ) );
		PostMessage(GetVParent(), msg);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void PropertySheet::OnFindDefaultButton()
{
    if (GetVParent())
    {
        PostMessage(GetVParent(), new KeyValues("FindDefaultButton"));
    }
}

bool PropertySheet::PageHasContextMenu( Panel *page ) const
{
	int pageNum = FindPage( page );
	if ( pageNum == m_Pages.InvalidIndex() )
		return false;

	return m_Pages[ pageNum ].contextMenu;
}

void PropertySheet::OnPanelDropped( CUtlVector< KeyValues * >& msglist )
{
	if ( msglist.Count() != 1 )
	{
		return;
	}

	PropertySheet *sheet = IsDroppingSheet( msglist );
	if ( !sheet )
	{
		// Defer to active page
		if ( _activePage && _activePage->IsDropEnabled() )
		{
			return _activePage->OnPanelDropped( msglist );
		}
		return;
	}

	KeyValues *data = msglist[ 0 ];

	Panel *page = reinterpret_cast< Panel * >( data->GetPtr( "propertypage" ) );
	char const *title = data->GetString( "tabname", "" );
	if ( !page || !sheet )
		return;

	// Can only create if sheet was part of a ToolWindow derived object
	ToolWindow *tw = dynamic_cast< ToolWindow * >( sheet->GetParent() );
	if ( tw )
	{
		IToolWindowFactory *factory = tw->GetToolWindowFactory();
		if ( factory )
		{
			bool showContext = sheet->PageHasContextMenu( page );
			sheet->RemovePage( page );
			if ( sheet->GetNumPages() == 0 )
			{
				tw->MarkForDeletion();
			}

			AddPage( page, title, NULL, showContext );
		}
	}
}

bool PropertySheet::IsDroppable( CUtlVector< KeyValues * >& msglist )
{
	//if ( !m_bDraggableTabs )
		return false;

	if ( msglist.Count() != 1 )
	{
		return false;
	}

	int mx, my;
	input()->GetCursorPos( mx, my );
	ScreenToLocal( mx, my );

	int tabHeight = IsSmallTabs() ? m_iTabHeightSmall : m_iTabHeight;
	if ( my > tabHeight )
		return false;

	PropertySheet *sheet = IsDroppingSheet( msglist );
	if ( !sheet )
	{
		return false;
	}

	if ( sheet == this )
		return false;

	return true;
}

// Mouse is now over a droppable panel
void PropertySheet::OnDroppablePanelPaint( CUtlVector< KeyValues * >& msglist, CUtlVector< Panel * >& dragPanels )
{
	// Convert this panel's bounds to screen space
	int x, y, w, h;

	GetSize( w, h );

	int tabHeight = IsSmallTabs() ? m_iTabHeightSmall : m_iTabHeight;
	h = tabHeight + 4;

	x = y = 0;
	LocalToScreen( x, y );

	surface()->DrawSetColor( GetDropFrameColor() );
	// Draw 2 pixel frame
	surface()->DrawOutlinedRect( x, y, x + w, y + h );
	surface()->DrawOutlinedRect( x+1, y+1, x + w-1, y + h-1 );

	if ( !IsDroppable( msglist ) )
	{
		return;
	}

	if ( !_showTabs )
	{
		return;
	}

	// Draw a fake new tab...

	x = 0;
	y = 2;
	w = 1;
	h = tabHeight;

	int last = m_PageTabs.Count();
	if ( last != 0 )
	{
		m_PageTabs[ last - 1 ]->GetBounds( x, y, w, h );
	}

	// Compute left edge of "fake" tab

	x += ( w + 1 );

	// Compute size of new panel
	KeyValues *data = msglist[ 0 ];
	char const *text = data->GetString( "tabname", "" );
	Assert( text );

	PageTab *fakeTab = new PageTab( this, "FakeTab", text, NULL, _tabWidth, NULL, false );
	fakeTab->SetBounds( x, 4, w, tabHeight - 4 );
	fakeTab->SetFont( m_tabFont );
	SETUP_PANEL( fakeTab );
	fakeTab->Repaint();
	surface()->SolveTraverse( fakeTab->GetVPanel(), true );
	surface()->PaintTraverse( fakeTab->GetVPanel() );
	delete fakeTab;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
void PropertySheet::SetKBNavigationEnabled( bool state )
{
	m_bKBNavigationEnabled = state;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool PropertySheet::IsKBNavigationEnabled() const
{
	return m_bKBNavigationEnabled;
}
