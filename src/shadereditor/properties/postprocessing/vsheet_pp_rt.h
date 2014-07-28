#ifndef CSHEET_PP_RT_H
#define CSHEET_PP_RT_H

#include "vSheets.h"

class CSheet_PP_RT : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_PP_RT, CSheet_Base );

	CSheet_PP_RT(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_PP_RT();

	virtual const char *GetSheetTitle(){return"Rendertarget";};

	void Paint();

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );

protected:

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:

	int m_iVguiPaint_Texture;
	Panel *m_pImgRef;

	//TextEntry *m_pText_RTName;
	ComboBox *m_pCBox_RTName;
};

#endif