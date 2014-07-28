#ifndef CSHEET_GENERAL_H
#define CSHEET_GENERAL_H

#include "vSheets.h"


class CSheet_General : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_General, CSheet_Base );

	CSheet_General(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_General();

	virtual const char *GetSheetTitle(){return"General";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	//MESSAGE_FUNC( OnPageShow, "PageShow" );
	//MESSAGE_FUNC( OnPageHide, "PageHide" );

protected:
	void PerformLayout();

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:

	TextEntry *m_pText_Name;
	CheckButton *m_pCheck_Preview;

	Label *m_pLabel_Name;
	Label *m_pLabel_Info;
	Label *m_pLabel_Example;
};


#endif