#ifndef CDIALOG_CFG_H
#define CDIALOG_CFG_H

#include "editorCommon.h"
#include "vgui_controls/controls.h"


class CDialog_GeneralConfig : public vgui::Frame
{
public:
	DECLARE_CLASS_SIMPLE( CDialog_GeneralConfig, Frame );

	CDialog_GeneralConfig( CNodeView *n, Panel *parent );
	~CDialog_GeneralConfig();

protected:

	void OnCommand( const char *cmd );

	void PerformLayout();

	CNodeView *pNodeView;
	vgui::RadioButton *m_pRadBut_Sm2;
	vgui::RadioButton *m_pRadBut_Sm3;
	vgui::CheckButton *m_pCBut_DepthTest;
	vgui::CheckButton *m_pCBut_DepthWrite;
	vgui::CheckButton *m_pCBut_sRGBWrite;
	vgui::ComboBox *m_pCBox_AlphaBlend;
	vgui::ComboBox *m_pCBox_Cullmode;

	void OnSave();

	//void OnDataLoaded( int mode );
};

#endif