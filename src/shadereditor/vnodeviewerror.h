#ifndef C_NODE_VIEW_ERROR_H
#define C_NODE_VIEW_ERROR_H


#include "cbase.h"
#include <vgui_controls/label.h>

using namespace vgui;


class CNodeViewError : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CNodeViewError, vgui::Panel );

public:

	enum
	{
		NVERROR_TEXTURE_SAMPLER_PARAM_MISMATCH = 0,
		NVERROR_RENDER_VIEW_SCENE_NOT_AT_START,
#ifdef SHADER_EDITOR_DLL_2006
		NVERROR_UNSUPPORTED_BY_2006,
#endif
		NVERROR_COUNT,
	};

	CNodeViewError( Panel *parent, int errorType );
	~CNodeViewError();

	const int GetErrorType();

	const bool IsAnimating();
	const bool IsClosing();

	void DoOpen();
	void DoClose();

	void SetText( const char *szParam );

protected:

	void PerformLayout();
	void ApplySchemeSettings(vgui::IScheme *pScheme);

	void Paint();


private:

	Label *pL;

	bool m_bClosing;
	int m_iErrorType;

	int m_iTallCur;
	int m_iTallGoal;
	int m_iTallMax;
	int m_iWide;

};

#endif