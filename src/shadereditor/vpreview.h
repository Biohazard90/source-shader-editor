#ifndef CHLSL_PREVIEW_H
#define CHLSL_PREVIEW_H

#include "cbase.h"
#include "editorCommon.h"
#include "cSolverCallback.h"

class CPreview : public CBaseDiag
{
	DECLARE_CLASS_SIMPLE( CPreview, CBaseDiag );

public:
	CPreview( vgui::Panel *parent, CNodeView *nodeview );
	~CPreview();

	virtual void Activate();

	void RememberPosition();
	void RestorePosition();

	void	OnSceneRender();
	void	OnPostRender();

	const char *GetEnvmap(); //{ return m_szEnvmap; };
	void SetEnvmap( const char *pszPath );

	void UpdateLayout();

protected:

	virtual void Paint();

	void OnCommand(const char* pcCommand);

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();

	virtual void OnFinishedClose();

	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );

	MESSAGE_FUNC_PARAMS( OnFileSelected, "FileSelected", pKV );
	MESSAGE_FUNC_PARAMS( OnCheckButtonChecked, "CheckButtonChecked", pKV );

	MESSAGE_FUNC_PARAMS( OnColorSelected, "ColorSelected", pKV );

private:
	CRenderPanel *pRenderPanel;
	int _px, _py, _sx, _sy;

	void InitColors();
	Color _col_BorderBright;
	Color _col_BorderDark;

	ComboBox *m_pCBox_RenderMode;
	ComboBox *m_pCBox_Model_Sequence;
	CheckButton *m_pCheck_MatOverride;
	CheckButton *m_pCheck_DoPPE;

	DHANDLE< FileOpenDialog >	m_hBrowser;
	void KillBrowser();
	void OpenBrowser( bool bPreviewMaterial );

	void LoadModel( const char *localPath );
	enum SeqFindMode_t
	{
		SEQFIND_EXACT = 0,
		SEQFIND_STARTSWITH,
		SEQFIND_CONTAINS,
	};
	int FindSequenceByName( const char *name, SeqFindMode_t mode = SEQFIND_CONTAINS );
	char **psz_SequenceList;
	int m_iNumSequences;

	void DoLayout_Empty();
	void DoLayout_Model();
	void DoLayout_PPE();

	//char m_szEnvmap[MAX_PATH];
};


#endif