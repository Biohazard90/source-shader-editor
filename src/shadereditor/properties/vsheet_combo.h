#ifndef CSHEET_COMBO_H
#define CSHEET_COMBO_H

#include "vSheets.h"


class CSheet_Combo : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Combo, CSheet_Base );

	CSheet_Combo(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Combo();

	virtual const char *GetSheetTitle(){return"Combo";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_Condition;
	TextEntry *m_pText_ComboName;
	TextEntry *m_pText_ComboVar;
	CheckButton *m_pCheck_Static;
};


#endif