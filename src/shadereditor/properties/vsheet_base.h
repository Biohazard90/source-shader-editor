#ifndef CSHEET_BASE_H
#define CSHEET_BASE_H

#include "vSheets.h"


class CSheet_Base : public PropertyPage
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_Base, PropertyPage );

	CSheet_Base(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	virtual ~CSheet_Base();

	CBaseNode *GetNode();
	KeyValues *GetData();

	//virtual const char *GetSheetTitle(){return"base";};
	virtual const char *GetSheetTitle(){Assert(szDynamicTitle); return szDynamicTitle;};
	void SetDynamicTitle( const char *name );

	virtual bool RequiresReset(){ return false; };

protected:
	CNodeView *pView;

	CBaseNode *n;
	KeyValues *pData;

	char *szDynamicTitle;

private:
	CSheet_Base( const CSheet_Base &other );
};


#endif