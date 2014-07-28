#ifndef CSHEET_PARALLAX_H
#define CSHEET_PARALLAX_H

#include "vSheets.h"
#include "editorcommon.h"


class CSheet_Parallax : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Parallax, CSheet_Base );

	CSheet_Parallax(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Parallax();

	virtual const char *GetSheetTitle(){return"Parallax UV";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );
	MESSAGE_FUNC_PARAMS( OnCheckButtonChecked, "CheckButtonChecked", pData );

private:

	void UpdateLayout( Panel *pCaller );

	TextEntry *pTEntry_MinSamples;
	TextEntry *pTEntry_MaxSamples;
	TextEntry *pTEntry_BinarySamples;

	CheckButton *pCheck_DepthTest;
	CheckButton *pCheck_GradientRead;

	ComboBox *pCBox_PrlxMode;
};


#endif