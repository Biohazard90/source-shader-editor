#ifndef CHLSL_BASEDIAG_H
#define CHLSL_BASEDIAG_H

#include "cbase.h"
#include "editorCommon.h"
#include "cSolverCallback.h"

#include "vgui_controls/html.h"

class CBaseDiag : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CBaseDiag, vgui::Frame );

public:
	CBaseDiag( vgui::Panel *parent, CNodeView *nodeview, const char *pElementName, bool bDoAutoSorting = false );
	~CBaseDiag();

	virtual void OnThink();

protected:

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual void OnManualCursorEntered();
	virtual void OnManualCursorExited();

protected:
	bool m_bDoAutoSorting;
	CNodeView *pNodeView;

	Color c_1;
	Color c_2;

	void InternalSort();
	void InternalMoveToFront();
};


#endif