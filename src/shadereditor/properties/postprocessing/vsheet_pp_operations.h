#ifndef CSHEET_PP_OPERATIONS_H
#define CSHEET_PP_OPERATIONS_H

#include "vSheets.h"

class CSheet_PP_ClearBuff : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_PP_ClearBuff, CSheet_Base );

	CSheet_PP_ClearBuff(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_PP_ClearBuff();

	virtual const char *GetSheetTitle(){return"Clear buffers";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

protected:

private:

	CheckButton *m_pCheck_ClearColor;
	CheckButton *m_pCheck_ClearDepth;

	TextEntry *m_pText_Values;
};

#endif