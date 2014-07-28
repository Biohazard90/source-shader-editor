#ifndef PROMPTSIMPLE_H
#define PROMPTSIMPLE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <UtlVector.h>

namespace vgui
{

class PromptSimple : public Frame
{
	DECLARE_CLASS_SIMPLE( PromptSimple, Frame );

public:
	PromptSimple(Panel *parent, const char *title);
	~PromptSimple();

	virtual void SetButtonMinSizeX( int &x );
	virtual void AddButton( const char *name, const char *cmd = NULL );
	virtual void AddButton( const char *name, KeyValues *pKVMessage );

	virtual void SetText( const char *text );

	virtual void QuickCenterLayout();

protected:
	virtual void OnSizeChanged(int wide, int tall);

	virtual void PerformLayout();
	MESSAGE_FUNC_PARAMS( WrappedCommand, "WrappedCommand", pKV );
	virtual void OnCommand( const char *cmd );

private:

	Button *AddButtonInternal( const char *name );

	char *_text;
	Label *_pLabel_text;

	CUtlVector< Button* > _hButtons;

	int _button_size_x;
};

}


#endif