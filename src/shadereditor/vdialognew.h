#ifndef CDIALOG_NEW_H
#define CDIALOG_NEW_H

#include "editorCommon.h"
#include "vgui_controls/controls.h"

enum
{
	CANVASINIT_SCRATCH = 0,
	CANVASINIT_LIGHTMAPPED_SIMPLE,
	CANVASINIT_LIGHTMAPPED_BUMP,
	CANVASINIT_MODEL_SIMPLE,
	CANVASINIT_MODEL_COMPLEX,
	CANVASINIT_POSTPROC,
	CANVASINIT_PPEFFECT,
	CANVASINIT_,
};

class CDialog_NewCanvas : public vgui::Frame
{
public:
	DECLARE_CLASS_SIMPLE( CDialog_NewCanvas, Frame );

	CDialog_NewCanvas( CNodeView *n, Panel *parent );
	~CDialog_NewCanvas();

protected:

	void OnCommand( const char *cmd );

	void PerformLayout();

	CNodeView *pNodeView;
	vgui::RadioButton *m_pRadBut_Sm2;
	vgui::RadioButton *m_pRadBut_Sm3;

	//vgui::TextEntry		*m_pShaderName;

	void OnDataLoaded( int mode );

	//char shadername[MAX_PATH];
};

#endif