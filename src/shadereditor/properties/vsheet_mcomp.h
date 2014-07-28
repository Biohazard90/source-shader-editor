#ifndef CSHEET_MCOMP_H
#define CSHEET_MCOMP_H

#include "vSheets.h"


class CSheet_MComp : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_MComp, CSheet_Base );

	CSheet_MComp(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_MComp();

	virtual const char *GetSheetTitle(){return"Matrix compose";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_MType;
	CheckButton *m_pCheck_Columns;
};


#endif