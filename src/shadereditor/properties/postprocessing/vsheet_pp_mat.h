#ifndef CSHEET_PP_MAT_H
#define CSHEET_PP_MAT_H

#include "vSheets.h"

#define DEFAULT_VMT_CONTENT "\"EDITOR_SHADER\"\n{\n\t\"$SHADERNAME\"\t\"\"\n}"

class CSheet_PP_Mat : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_PP_Mat, CSheet_Base );

	CSheet_PP_Mat(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_PP_Mat();

	virtual const char *GetSheetTitle(){return"Material";};

	virtual bool RequiresReset(){ return m_bPerformedInjection; };

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );
	MESSAGE_FUNC( OnRestoreNode, "RestoreNode" );

	MESSAGE_FUNC_PARAMS( OnFileSelected, "FileSelected", pKV );
	MESSAGE_FUNC_PARAMS( CheckButtonChecked, "CheckButtonChecked", pKV );
	//MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );

	MESSAGE_FUNC_PARAMS( OnCodeUpdate, "CodeUpdate", pKV );
protected:

	virtual void OnCommand( const char *cmd );

private:

	void WriteDataToKV( KeyValues *pKV );
	bool m_bPerformedInjection;

	char m_szMatPath[MAX_PATH*4];
	KeyValues *m_pVMTBuffer;

	DHANDLE< FileOpenDialog >	m_hMaterialBrowser;
	void OpenFiledialog( bool bSave );
	void DoSave();
	void DoLoad();

	CheckButton *m_pCheck_Inline;

	Button *m_pLoadMaterial;
	Button *m_pSaveMaterial;
	Label *m_pLabel_MatPath;

};

#endif