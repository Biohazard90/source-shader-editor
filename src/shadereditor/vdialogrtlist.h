#ifndef DIALOG_RT_LIST_H
#define DIALOG_RT_LIST_H

#include "editorCommon.h"
#include "vgui_controls/controls.h"


class CDialog_RendertargetList : public CBaseDiag
{
public:
	DECLARE_CLASS_SIMPLE( CDialog_RendertargetList, CBaseDiag );

	CDialog_RendertargetList( Panel *parent );
	~CDialog_RendertargetList();

protected:

	void OnCommand( const char *cmd );

	void PerformLayout();

	MESSAGE_FUNC( OnUpdateList, "UpdateList" );

private:

	PanelListPanel *m_pList_RT;

	void FillList();

};


class CDialog_RendertargetSettings : public CBaseDiag //vgui::Frame
{
public:
	DECLARE_CLASS_SIMPLE( CDialog_RendertargetSettings, CBaseDiag );

	CDialog_RendertargetSettings( Panel *parent );
	~CDialog_RendertargetSettings();

	void InitFromRT( RTDef *rt );
	void WriteToRT( RTDef *rt );
	void SetDefaults();

protected:

	void OnCommand( const char *cmd );

	void PerformLayout();

	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );

	virtual void CloseModal();

	bool DoesRTExist( const char *name );
	void ShowErrorDiag( const char *name );

private:

	TextEntry *m_pText_RTName;
	TextEntry *m_pText_SizeX;
	TextEntry *m_pText_SizeY;

	PanelListPanel *m_pList_Flags;

	ComboBox *m_pCBox_ImgFormat;
	ComboBox *m_pCBox_SizeMode;
	ComboBox *m_pCBox_DepthMode;

	CUtlVector< CheckButton* > m_hFlag_Buttons;

	RTDef *m_pRTTarget;
};



#endif