#ifndef C_DIALOG_RECOMPILE_ALL_H
#define C_DIALOG_RECOMPILE_ALL_H

#include "cbase.h"
#include "editorCommon.h"

struct DumpCompileHelper_t
{
public:

	DumpCompileHelper_t();
	~DumpCompileHelper_t();

	BasicShaderCfg_t *shaderDump;
	int iCompileStep;

private:
	DumpCompileHelper_t( const DumpCompileHelper_t &o );
};

class CDialogRecompileAll : public CBaseDiag
{
	DECLARE_CLASS_SIMPLE( CDialogRecompileAll, CBaseDiag );

public:
	CDialogRecompileAll( vgui::Panel *parent );
	~CDialogRecompileAll();

protected:
	void OnCommand( const char *cmd );
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();

	void Paint();

	void OnThink();

private:
	CUtlVector<DumpCompileHelper_t*> hCompileList;
	bool m_bCompilerRunning;

	void LogMsg( const char *msg, ... );
	void LogMsg( Color textCol, const char *msg, ... );
	RichText *m_pRichText_Log;

	Button *m_pClose;
	void SetButtonState( bool bShowClose );

	CNodeView *m_pView;
	Panel *m_pProgressPos;
	bool m_bAutoInjectOld;
};


#endif