#ifndef CSHEET_PP_DRAWMAT_H
#define CSHEET_PP_DRAWMAT_H

#include "vSheets.h"

class CSheet_PP_DrawMat : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_PP_DrawMat, CSheet_Base );

	CSheet_PP_DrawMat(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_PP_DrawMat();

	virtual const char *GetSheetTitle(){return"Draw material";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_SizingTg;
	ComboBox *m_pCBox_SizingSrc;
	CheckButton *m_pCheck_PushTarget;

	//TextEntry *m_pManualSourceUV_x;
	//TextEntry *m_pManualSourceUV_y;

	TextEntry *m_pTE_Destx;
	TextEntry *m_pTE_Desty;
	TextEntry *m_pTE_Destw;
	TextEntry *m_pTE_Desth;

	TextEntry *m_pTE_Srcx0;
	TextEntry *m_pTE_Srcy0;
	TextEntry *m_pTE_Srcx1;
	TextEntry *m_pTE_Srcy1;

	TextEntry *m_pTE_Srcw;
	TextEntry *m_pTE_Srch;
};

#endif