#ifndef CSHEET_LOOP_H
#define CSHEET_LOOP_H

#include "vSheets.h"


class CSheet_Loop : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Loop, CSheet_Base );

	CSheet_Loop(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Loop();

	virtual const char *GetSheetTitle(){return"Loop";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_Condition;
	TextEntry *m_pText_Start;
	TextEntry *m_pText_End;
};


#endif