#ifndef C_EDITOR_FG_PAGE_H
#define C_EDITOR_FG_PAGE_H

#include "vSheets.h"


class CFlowGraphPage : public PropertyPage
{
public:
	DECLARE_CLASS_SIMPLE( CFlowGraphPage, PropertyPage );

	CFlowGraphPage( Panel *parent );
	~CFlowGraphPage();

	void SetFlowGraph( CNodeView *p );
	CNodeView *GetFlowGraph(){ return pGraph; };

	MESSAGE_FUNC( PageShow, "PageShow" );

protected:

	CNodeView *pGraph;
};


#endif