#ifndef CSHEET_FINAL_H
#define CSHEET_FINAL_H

#include "vSheets.h"


class CSheet_Final : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Final, CSheet_Base );

	CSheet_Final(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Final();

	virtual const char *GetSheetTitle(){return"Final output";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_Tonemap;
	CheckButton *m_pCheck_WriteDepth;
};


#endif