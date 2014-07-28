#ifndef CSHEET_CMATRIX_H
#define CSHEET_CMATRIX_H

#include "vSheets.h"


class CSheet_CMatrix : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_CMatrix, CSheet_Base );

	CSheet_CMatrix(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_CMatrix();

	virtual const char *GetSheetTitle(){return"Custom matrix";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_MType;
};


#endif