#ifndef CSHEET_LIGHTSCALE_H
#define CSHEET_LIGHTSCALE_H

#include "vSheets.h"


class CSheet_Lightscale : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Lightscale, CSheet_Base );

	CSheet_Lightscale(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Lightscale();

	virtual const char *GetSheetTitle(){return"Lightscale";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_Lightscale;
};


#endif