#ifndef CSHEET_CONSTANT_H
#define CSHEET_CONSTANT_H

#include "vSheets.h"


class CSheet_Constant : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Constant, CSheet_Base );

	CSheet_Constant(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Constant();

	virtual const char *GetSheetTitle(){return"Constant";};

	void OnCommand( const char *cmd );

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	TextEntry *m_pText_Values;
};

class CSheet_Random : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Random, CSheet_Base );

	CSheet_Random(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Random();

	virtual const char *GetSheetTitle(){return"Random";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_NumComp;
	TextEntry *m_pTEntry_ValueMin;
	TextEntry *m_pTEntry_ValueMax;
};



class CSheet_EnvCTexelsize : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_EnvCTexelsize, CSheet_Base );

	CSheet_EnvCTexelsize(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_EnvCTexelsize();

	virtual const char *GetSheetTitle(){return"Framebuffer texelsize";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_FBScaling;
};


#endif