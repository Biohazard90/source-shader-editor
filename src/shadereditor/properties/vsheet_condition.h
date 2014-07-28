#ifndef CSHEET_CONDITION_H
#define CSHEET_CONDITION_H

#include "vSheets.h"


class CSheet_Condition : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Condition, CSheet_Base );

	CSheet_Condition(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Condition();

	virtual const char *GetSheetTitle(){return"Condition";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_Condition;
};


#endif