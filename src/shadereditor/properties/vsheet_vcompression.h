#ifndef CSHEET_VCOMPRESSION_H
#define CSHEET_VCOMPRESSION_H

#include "vSheets.h"


class CSheet_VCompression : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_VCompression, CSheet_Base );

	CSheet_VCompression(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_VCompression();

	virtual const char *GetSheetTitle(){return"Vertex decompress";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

private:

	ComboBox *m_pCBox_CompressionModes;
};


#endif