#ifndef CSHEET_TEXTRANSFORM_H
#define CSHEET_TEXTRANSFORM_H

#include "vSheets.h"


class CSheet_TexTransform : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_TexTransform, CSheet_Base );

	CSheet_TexTransform(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_TexTransform();

	virtual const char *GetSheetTitle(){return"Texture transform";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	CheckButton *m_pCheck_Center;
	CheckButton *m_pCheck_Rotation;
	CheckButton *m_pCheck_Scale;
	CheckButton *m_pCheck_Translation;
};


#endif