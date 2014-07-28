#ifndef CSHEET_MULTIPLY_H
#define CSHEET_MULTIPLY_H

#include "vSheets.h"


class CSheet_Multiply : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Multiply, CSheet_Base );

	CSheet_Multiply(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Multiply();

	virtual const char *GetSheetTitle(){return"Multiply";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	CheckButton *m_pCheck_MatrixAsRotation;
};


#endif