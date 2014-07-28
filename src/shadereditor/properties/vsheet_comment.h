#ifndef CSHEET_COMMENT_H
#define CSHEET_COMMENT_H

#include "vSheets.h"


class CSheet_Comment : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Comment, CSheet_Base );

	CSheet_Comment(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_Comment();

	virtual const char *GetSheetTitle(){return"Group";};

	void OnCommand( const char *cmd );

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	TextEntry *m_pText_Values;
};


#endif