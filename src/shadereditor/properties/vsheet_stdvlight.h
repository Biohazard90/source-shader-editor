#ifndef CSHEET_STD_VLIGHT_H
#define CSHEET_STD_VLIGHT_H

#include "vSheets.h"


class CSheet_Std_VLight : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Std_VLight, CSheet_Base );

	CSheet_Std_VLight(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Std_VLight();

	virtual const char *GetSheetTitle(){return"Vertex lighting";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	MESSAGE_FUNC_PARAMS( OnCheckButtonChecked, "CheckButtonChecked", pData );

private:

	CheckButton *m_pCheck_StaticLighting;
	CheckButton *m_pCheck_Halflambert;
};


#endif