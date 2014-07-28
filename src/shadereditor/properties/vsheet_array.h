#ifndef CSHEET_ARRAY_H
#define CSHEET_ARRAY_H

#include "vSheets.h"


class CSheet_Array : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Array, CSheet_Base );

	CSheet_Array(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Array();

	virtual const char *GetSheetTitle(){return"Array";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	MESSAGE_FUNC_PARAMS( OnTextFocusLost, "TextKillFocus", pKV );
	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );

	void OnCommand( const char *cmd );
	
private:

	char ***BuildStringSheet( int &sizex, int &sizey );
	void DestroyStringSheet( char ***strings, const int &sizex, const int &sizey );

	void UpdateArrayBlocks( const int &dest_x, const int &dest_y );

	TextEntry *m_pTEntry_Size_X;
	TextEntry *m_pTEntry_Size_Y;

	TextEntry *m_pTEntry_Random_Min;
	TextEntry *m_pTEntry_Random_Max;
	TextEntry *m_pTEntry_Gauss_Bias;

	ComboBox *m_pCBox_Datatype;

	PanelListPanel *m_pArrayPanel;
	CUtlVector< CUtlVector< TextEntry* >* >m_hArray_Y_Major;
};


#endif