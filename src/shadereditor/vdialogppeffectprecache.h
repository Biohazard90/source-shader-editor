#ifndef DIALOG_PPE_PRECACHE_H
#define DIALOG_PPE_PRECACHE_H

#include "cbase.h"
#include "editorCommon.h"

class CDialog_PPEPrecache : public CBaseDiag
{
public:
	DECLARE_CLASS_SIMPLE( CDialog_PPEPrecache, CBaseDiag );

	CDialog_PPEPrecache( Panel *parent );
	~CDialog_PPEPrecache();

protected:

	MESSAGE_FUNC_PARAMS( OnFileSelected, "FileSelected", pKV );
	MESSAGE_FUNC_PARAMS( CheckButtonChecked, "CheckButtonChecked", pKV );

	virtual void OnCommand( const char *cmd );

private:

	void FillList();
	void OpenFiledialog();

	DHANDLE< FileOpenDialog > m_hEffectBrowser;
	PanelListPanel *m_pList_Effects;

};


#endif