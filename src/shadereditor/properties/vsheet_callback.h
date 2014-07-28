#ifndef CSHEET_CALLBACK_H
#define CSHEET_CALLBACK_H

#include "vSheets.h"


class CSheet_Callback : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Callback, CSheet_Base );

	CSheet_Callback(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Callback();

	virtual const char *GetSheetTitle(){return"Callback";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_Callbacks;
};


#endif