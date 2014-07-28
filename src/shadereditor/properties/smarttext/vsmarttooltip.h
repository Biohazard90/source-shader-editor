#ifndef SMART_TOOLTIP_H
#define SMART_TOOLTIP_H

#include "cbase.h"
#include "vgui_controls/controls.h"
#include <vgui_controls/panel.h>

using namespace vgui;

class CSmartObject;

class CSmartTooltip : public Panel
{
	DECLARE_CLASS_SIMPLE( CSmartTooltip, Panel );
public:

	enum
	{
		STTIPMODE_NORMAL = 0,
		STTIPMODE_FUNCTIONPARAMS
	};

	CSmartTooltip( Panel *parent, const char *pElementname );
	~CSmartTooltip();

	bool Init( CSmartObject *pO, int iMode = STTIPMODE_NORMAL );

	void SetActiveParam( int iP );

	void Paint();
	void ApplySchemeSettings(IScheme *pScheme);

	bool IsFunctionParams();

private:
	int GetNumRowsForString( const char *pStr, int *max_x = NULL );

	CSmartObject *m_pCurObject;

	int m_iFont;

	int m_iMode;
	int m_iParamHighlight;
};

#endif