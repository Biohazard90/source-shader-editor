#ifndef CSHEET_PP_RENDERVIEW_H
#define CSHEET_PP_RENDERVIEW_H

#include "vSheets.h"

class CSheet_PP_RenderView : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_PP_RenderView, CSheet_Base );

	CSheet_PP_RenderView(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_PP_RenderView();

	virtual const char *GetSheetTitle(){return"Render view";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );

protected:

	//virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();

private:

	void FillOptions();
	CUtlVector< CheckButton* >	hOptions_Bool;
	CUtlVector< TextEntry* >	hOptions_Int;
	CUtlVector< TextEntry* >	hOptions_Float;
	CUtlVector< TextEntry* >	hOptions_String;

	ComboBox *m_pCBox_VrNames;
	PanelListPanel *m_pOptionsList;
	CheckButton *m_pCheck_Scene;
};

#endif