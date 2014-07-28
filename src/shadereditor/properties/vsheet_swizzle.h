#ifndef CSHEET_SWIZZLE_H
#define CSHEET_SWIZZLE_H

#include "vSheets.h"


class CSheet_Swizzle : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Swizzle, CSheet_Base );

	CSheet_Swizzle(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Swizzle();

	virtual const char *GetSheetTitle(){return"Swizzle";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	void OnCommand( const char *cmd );

private:

	TextEntry *m_pText_Values;
};


#endif