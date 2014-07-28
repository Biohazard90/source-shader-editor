#ifndef CSHEET_PSOUTPUT_H
#define CSHEET_PSOUTPUT_H

#include "vSheets.h"

class CSheet_PSOutput : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_PSOutput, CSheet_Base );

	CSheet_PSOutput(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_PSOutput();

	virtual const char *GetSheetTitle(){return"PS Output";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	MESSAGE_FUNC_INT( OnSliderMoved, "SliderMoved", position );

	//MESSAGE_FUNC_CHARPTR( OnComboboxClose, "OnMenuClose", szName );

private:

	Slider *m_pSlider_col;
	CheckButton *m_pCheck_Depth;

	Label *m_pLabel_Info_Num_Colors;
};


#endif