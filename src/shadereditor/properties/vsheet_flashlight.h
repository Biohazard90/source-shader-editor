#ifndef CSHEET_FLASHLIGHT_H
#define CSHEET_FLASHLIGHT_H

#include "vSheets.h"


class CSheet_Flashlight : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Flashlight, CSheet_Base );

	CSheet_Flashlight(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Flashlight();

	virtual const char *GetSheetTitle(){return"Flashlight";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	CheckButton *m_pCheck_Specular;
};


#endif