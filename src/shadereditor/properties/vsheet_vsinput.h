#ifndef CSHEET_VSINPUT_H
#define CSHEET_VSINPUT_H

#include "vSheets.h"

class CSheet_VSInput : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_VSInput, CSheet_Base );

	CSheet_VSInput(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_VSInput();

	virtual const char *GetSheetTitle(){return"VS Input";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	MESSAGE_FUNC_INT( OnSliderMoved, "SliderMoved", position );
	MESSAGE_FUNC_PARAMS( OnCheckButtonChecked, "CheckButtonChecked", pData );

private:

	CheckButton *m_pCheck_flex;
	CheckButton *m_pCheck_normal;
	CheckButton *m_pCheck_normal_compression;
	CheckButton *m_pCheck_tangent;
	CheckButton *m_pCheck_tangent_skinable;
	CheckButton *m_pCheck_blend;

	Slider *m_pSlider_tex;
	Slider *m_pSlider_col;

	Label *m_pLabel_Info_Num_TexCoords;
	Label *m_pLabel_Info_Num_Colors;

	ComboBox *m_pCBox_DType_TexCoords[3];
	ComboBox *m_pCBox_DType_Color[2];
};


#endif