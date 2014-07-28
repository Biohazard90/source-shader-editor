#ifndef CDIALOG_EDITOR_CFG_H
#define CDIALOG_EDITOR_CFG_H

#include "editorCommon.h"
#include "vgui_controls/controls.h"


class CDialog_EditorConfig : public vgui::Frame
{
public:
	DECLARE_CLASS_SIMPLE( CDialog_EditorConfig, Frame );

	CDialog_EditorConfig( Panel *parent );
	~CDialog_EditorConfig();

protected:

	void OnCommand( const char *cmd );

	void PerformLayout();

	CNodeView *pNodeView;
	vgui::CheckButton *m_pCBut_Draw_Datatypes;
	vgui::CheckButton *m_pCBut_Draw_Shadows;
	vgui::CheckButton *m_pCBut_Draw_Info;
	vgui::CheckButton *m_pCBut_Draw_Tooltip;

	vgui::CheckButton *m_pCBut_Compile_Preview;
	vgui::CheckButton *m_pCBut_Compile_Full;
	vgui::CheckButton *m_pCBut_Compile_Publish;
	vgui::CheckButton *m_pCBut_Compile_WarnOnSave;

	void OnSave();
};

#endif