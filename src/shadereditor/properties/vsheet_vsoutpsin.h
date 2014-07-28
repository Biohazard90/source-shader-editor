#ifndef CSHEET_PSINPUT_H
#define CSHEET_PSINPUT_H

#include "vSheets.h"

class CSheet_PSInVSOut : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_PSInVSOut, CSheet_Base );

	CSheet_PSInVSOut(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_PSInVSOut();

	virtual const char *GetSheetTitle(){return"VS Output / PS Input";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	MESSAGE_FUNC_INT( OnSliderMoved, "SliderMoved", position );

	//MESSAGE_FUNC_CHARPTR( OnComboboxClose, "OnMenuClose", szName );
	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );

private:

	Slider *m_pSlider_tex;
	Slider *m_pSlider_col;

	PanelListPanel *m_pListTexcoords;

	Label *m_pLabel_List[VSTOPS_NUM_TEXCOORDS];
	ComboBox *m_pComboBox_List[VSTOPS_NUM_TEXCOORDS];
	TextEntry *m_pTextEntry_List[VSTOPS_NUM_TEXCOORDS];

	Label *m_pLabel_List_Colors[VSTOPS_NUM_COLORS];
	ComboBox *m_pComboBox_List_Colors[VSTOPS_NUM_COLORS];
	TextEntry *m_pTextEntry_List_Colors[VSTOPS_NUM_COLORS];

	void FillComboBox( ComboBox *c, int numItems );
	void EnableItems_Textures( int numColors );
	void EnableItems_Colors( int numTexcoords );

	Label *m_pLabel_Info_Num_TexCoords;
	Label *m_pLabel_Info_Num_Colors;
};


#endif