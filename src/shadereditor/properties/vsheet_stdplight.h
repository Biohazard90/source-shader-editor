#ifndef CSHEET_STD_PLIGHT_H
#define CSHEET_STD_PLIGHT_H

#include "vSheets.h"


class CSheet_Std_PLight : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Std_PLight, CSheet_Base );

	CSheet_Std_PLight(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Std_PLight();

	virtual const char *GetSheetTitle(){return"Pixelshader lighting";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	CheckButton *m_pCheck_StaticLighting;
	CheckButton *m_pCheck_AOTerm;
	CheckButton *m_pCheck_Halflambert;
};


#endif