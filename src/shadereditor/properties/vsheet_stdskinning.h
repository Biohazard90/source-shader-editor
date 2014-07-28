#ifndef CSHEET_STD_SKINNING_H
#define CSHEET_STD_SKINNING_H

#include "vSheets.h"


class CSheet_Std_Skinning : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Std_Skinning, CSheet_Base );

	CSheet_Std_Skinning(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Std_Skinning();

	//virtual const char *GetSheetTitle(){return"Skinning/ Morph";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBoxMode;
};


#endif