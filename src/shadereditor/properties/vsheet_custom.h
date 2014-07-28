#ifndef CSHEET_CUSTOM_H
#define CSHEET_CUSTOM_H

#include "vSheets.h"

struct __customCtrlEntry
{
	__customCtrlEntry();
	void Delete( bool killChildren = false );

	TextEntry *pName;
	ComboBox *pType;
	Button *pDel;
};

class CSheet_Custom_IO;

class CSheet_Custom : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Custom, CSheet_Base );

	CSheet_Custom(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Custom();

	virtual const char *GetSheetTitle(){return"Custom code";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );
	MESSAGE_FUNC( OnRestoreNode, "RestoreNode" );
	
	MESSAGE_FUNC_PARAMS( OnCommand, "Command", pKV );
	MESSAGE_FUNC_PARAMS( OnCodeUpdate, "CodeUpdate", pKV );
	//MESSAGE_FUNC_CHARPTR( OnCommand, "Command", command );

	//MESSAGE_FUNC_CHARPTR( OnComboboxClose, "OnMenuClose", szName );
	//MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );

	MESSAGE_FUNC_PARAMS( OnFileSelected, "FileSelected", pKV );
	MESSAGE_FUNC_PARAMS( CheckButtonChecked, "CheckButtonChecked", pKV );
	
	void SetIOPage( CSheet_Custom_IO *p );

	virtual bool RequiresReset();

private:
	KeyValues *m_pKVCodeCache;
	bool m_bPerformedInjection;

	void WriteDataToKV( KeyValues *pKV, bool bCheckFilePath = true );

	DHANDLE< FileOpenDialog >	m_hCodeBrowser;
	void OpenFiledialog( bool bSave );
	void DoSave();
	void DoLoad();

	char m_szFilePath[MAX_PATH*4];

	Label *m_pLabel_Path;
	TextEntry *m_pEntry_FuncName;

	vgui::CheckButton *m_pCheck_Inline;
	vgui::CheckButton *m_pCheck_Lighting_PS;
	vgui::CheckButton *m_pCheck_Lighting_VS;

	int BuildEnvDataFlags();

	CSheet_Custom_IO *m_pIO_Page;
};


class CSheet_Custom_IO : public CSheet_Base
{
public:
	friend class CSheet_Custom;

	DECLARE_CLASS_SIMPLE( CSheet_Custom_IO, CSheet_Base );

	CSheet_Custom_IO(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Custom_IO();

	virtual const char *GetSheetTitle(){return"Inputs/ Outputs";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	MESSAGE_FUNC_PARAMS( OnCommand, "Command", pKV );

private:

	void ClearIOs( KeyValues *pKV, int firstIn = 0, int firstOut = 0 );

	void WriteDataToKV( KeyValues *pKV );

	void PurgeNameVecs();

	__customCtrlEntry *AllocEntry( int idx, bool bOutput, __funcParamSetup *data, Panel **p );
	void AllocList();
	void AllocNodeData();

	PanelListPanel *m_pList_Jacks;

	void FillComboBox( ComboBox *c );

	CUtlVector< __funcParamSetup* >m_hszVarNames_In;
	CUtlVector< __funcParamSetup* >m_hszVarNames_Out;

	CUtlVector< __customCtrlEntry* >m_hCtrl_In;
	CUtlVector< __customCtrlEntry* >m_hCtrl_Out;
};


#endif