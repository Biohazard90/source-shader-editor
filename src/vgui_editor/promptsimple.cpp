#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/IBorder.h>
#include <vgui/ISurface.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include <vgui_controls/MenuBar.h>
#include <vgui_controls/MenuButton.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/promptsimple.h>

#include "vgui_editor_platform.h"

#include <tier0/memdbgon.h>

using namespace vgui;


PromptSimple::PromptSimple(Panel *parent, const char *title) :
	Frame(parent, "")
{
	_pLabel_text = new Label( this, "", "" );
	_text = NULL;

	_button_size_x = 48;

	SetSizeable( false );
	SetCloseButtonVisible( false );

	SetDeleteSelfOnClose( true );

	SetTitle( title, true );
	Activate();
	DoModal();

	SetSize( 200, 150 );
}

PromptSimple::~PromptSimple()
{
	delete [] _text;
	_hButtons.Purge();
}

void PromptSimple::SetButtonMinSizeX( int &x )
{
	_button_size_x = x;

	InvalidateLayout();
}
void PromptSimple::OnSizeChanged(int wide, int tall)
{
	BaseClass::OnSizeChanged( wide, tall );
	InvalidateLayout();
}

void PromptSimple::AddButton( const char *name, const char *cmd )
{
	Button *b = AddButtonInternal( name );

	if ( cmd != NULL )
	{
		b->SetCommand( cmd );
		b->AddActionSignalTarget( GetParent() );
	}
	else
		b->SetCommand( "autoclose" );
}

void PromptSimple::AddButton( const char *name, KeyValues *pKVMessage )
{
	Button *b = AddButtonInternal( name );

	KeyValues *pWrappedKV = new KeyValues( "WrappedCommand" );
	pWrappedKV->SetPtr( "wrappedcmd", pKVMessage );

	b->SetCommand( pWrappedKV );
}

Button *PromptSimple::AddButtonInternal( const char *name )
{
	Button *b = new Button( this, name, name );

	b->AddActionSignalTarget( this );
	b->RequestFocus();
	_hButtons.AddToTail( b );

	InvalidateLayout();

	return b;
}

void PromptSimple::WrappedCommand( KeyValues *pKV )
{
	KeyValues *pCmd = (KeyValues*)pKV->GetPtr( "wrappedcmd" );
	PostMessage( GetParent(), pCmd );
	Close();
}

void PromptSimple::OnCommand( const char *cmd )
{
	Close();

	//Frame::OnCommand(cmd);
}

void PromptSimple::SetText( const char *text )
{
	delete [] _text;
	int len = Q_strlen( text );
	_text = new char[ len + 1 ];
	Q_strcpy( _text, text );
	_text[ len ] = '\0';
	_pLabel_text->SetText( _text );

	InvalidateLayout();
}

void PromptSimple::QuickCenterLayout()
{
	int sx, sy;
	GetSize( sx, sy );

	_pLabel_text->SetContentAlignment( vgui::Label::a_center );
	_pLabel_text->SetWrap( false );
	_pLabel_text->SetPos( 0,0 );
	_pLabel_text->SetSize( sx, sy );
}

void PromptSimple::PerformLayout()
{
	Frame::PerformLayout();

	int INSET = 10;
	int INSET_YTOP = 20;
	int w, t;
	GetSize( w, t );
	int x = w;
	int y = 0;

	int maxbuttonY = 0;
	for ( int i = _hButtons.Count()-1; i >= 0 ; i-- )
	{
		Button *b = _hButtons[ i ];

		int b_w, b_t;
		b->GetSize( b_w, b_t );
		b_w = max( b_w, _button_size_x );
		maxbuttonY = max( maxbuttonY, b_t );
		b->SetSize( b_w, b_t );

		x -= b_w + INSET;
		y = t - b_t - INSET;

		b->SetPos( x, y );
	}

	_pLabel_text->SetPos( INSET, INSET_YTOP );
	_pLabel_text->SetSize( w - INSET * 2, t - INSET_YTOP - INSET * 2 - maxbuttonY );
	_pLabel_text->SetWrap( true );
	_pLabel_text->InvalidateLayout( true );

	if ( _text )
		_pLabel_text->SetText( _text );
}
