#ifndef CSHEET_VMT_PARAM_H
#define CSHEET_VMT_PARAM_H

#include "vSheets.h"


class CSheet_VParam_Mutable : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_VParam_Mutable, CSheet_Base );

	CSheet_VParam_Mutable(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_VParam_Mutable();

	virtual const char *GetSheetTitle(){return"VParam mutable";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	TextEntry *m_pText_Values;

	ComboBox *m_pCBox_Params;
	ComboBox *m_pCBox_Comps;
};


class CSheet_VParam_Static : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_VParam_Static, CSheet_Base );

	CSheet_VParam_Static(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_VParam_Static();

	virtual const char *GetSheetTitle(){return"VParam static";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	void OnCommand( const char *cmd );

private:

	TextEntry *m_pTEntry_Name;
	ComboBox *m_pCBox_Comps;

	TextEntry *m_pText_Values;
};


#endif