#ifndef CDIALOG_PPE_CFG_H
#define CDIALOG_PPE_CFG_H

#include "editorCommon.h"
#include "vgui_controls/controls.h"


class CDialog_PPEConfig : public vgui::Frame
{
public:
	DECLARE_CLASS_SIMPLE( CDialog_PPEConfig, Frame );

	CDialog_PPEConfig( Panel *parent, CNodeView *pNodeView );
	~CDialog_PPEConfig();

protected:

	void OnCommand( const char *cmd );

	//void PerformLayout();

private:

	void OnSave();

	vgui::CheckButton *m_pCheck_AutoUpdateFBCopy;

	CNodeView *m_pNodeView;
};

#endif