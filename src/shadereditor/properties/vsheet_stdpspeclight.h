#ifndef CSHEET_STD_PSPECLIGHT_H
#define CSHEET_STD_PSPECLIGHT_H

#include "vSheets.h"


class CSheet_Std_PSpecLight : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Std_PSpecLight, CSheet_Base );

	CSheet_Std_PSpecLight(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Std_PSpecLight();

	virtual const char *GetSheetTitle(){return"Pixelshader specular";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	CheckButton *m_pCheck_AOTerm;
};


#endif