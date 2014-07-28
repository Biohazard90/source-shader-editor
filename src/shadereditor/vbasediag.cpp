
#include "cbase.h"
#include "editorCommon.h"
//#include "vgui_int.h"

static CUtlVector< CBaseDiag* >hListDiags;

CBaseDiag::CBaseDiag( vgui::Panel *parent, CNodeView *nodeview, const char *pElementName, bool bDoAutoSorting ) : BaseClass( parent, pElementName )
{
	m_bDoAutoSorting = bDoAutoSorting;
	pNodeView = nodeview;

	Activate();
	SetVisible( true );

	SetPaintBackgroundEnabled(true);
	SetPaintEnabled(true);
	SetCloseButtonVisible(true);

	SetSizeable(true);
	SetMoveable(true);
	SetTitle("",true);
	SetMinimumSize( 400, 300 );

	SetDeleteSelfOnClose( true );

	if ( m_bDoAutoSorting )
		hListDiags.AddToHead(this);
}
CBaseDiag::~CBaseDiag()
{
	if ( m_bDoAutoSorting )
		hListDiags.FindAndRemove(this);
}

void CBaseDiag::OnManualCursorEntered()
{
	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);

	if ( m_bDoAutoSorting )
		InternalMoveToFront();
}
void CBaseDiag::OnManualCursorExited()
{
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
}

void CBaseDiag::InternalSort()
{
	VPANEL hfocus = g_pVGuiInput->GetFocus();

	for ( int i = 0; i < hListDiags.Count(); i++ )
	{
		CBaseDiag *d = hListDiags[i];
		const bool bFocus_is_parent = ipanel()->HasParent( d->GetVPanel(), hfocus );
		const bool bFocus_has_same_parent = d->GetParent() && ipanel()->HasParent( hfocus, d->GetParent()->GetVPanel() );
		const bool bFocus_is_child = ipanel()->HasParent( hfocus, d->GetVPanel() );
		const bool bGlobalInput = pEditorRoot && pEditorRoot->HasInputEnabled();

		if ( ( bFocus_is_parent || bFocus_has_same_parent ) && !bFocus_is_child && !g_pVGuiInput->GetAppModalSurface() &&
			!IsBuildModeActive() && bGlobalInput )
			d->MoveToFront();
	}
}

void CBaseDiag::InternalMoveToFront()
{
	hListDiags.FindAndRemove( this );
	hListDiags.AddToTail( this );
}

void CBaseDiag::OnThink(void)
{
	BaseClass::OnThink();

	SetBgColor( c_1 );
	SetOutOfFocusColor( c_2 );

	if ( m_bDoAutoSorting )
	{
		int a,b,c,d,e,f;
		GetBounds( a,b,c,d );
		input()->GetCursorPosition( e, f );
		ScreenToLocal( e, f );

		const bool bGlobalInput = pEditorRoot && pEditorRoot->HasInputEnabled();
		const bool bMWasActive = IsMouseInputEnabled();
		const bool bMShouldActive = e >= 0 && e <= c && f >= 0 && f <= d && bGlobalInput && (g_pVGuiInput->GetAppModalSurface() == GetVPanel() || !g_pVGuiInput->GetAppModalSurface());
		if ( bMWasActive != bMShouldActive && !IsBuildModeActive() )
		{
			if ( bMShouldActive )
				OnManualCursorEntered();
			else
				OnManualCursorExited();
		}

		InternalSort();
	}
}

void CBaseDiag::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	c_1 = pScheme->GetColor( "Grey_t", GetBgColor() );
	c_2 = pScheme->GetColor( "Grey_Faint_t", GetOutOfFocusColor() );
}
