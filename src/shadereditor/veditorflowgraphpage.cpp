
#include "cbase.h"
#include "editorcommon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CFlowGraphPage::CFlowGraphPage( Panel *parent ) : PropertyPage( parent, "" )
{
	pGraph = NULL;
}

CFlowGraphPage::~CFlowGraphPage()
{
}

void CFlowGraphPage::SetFlowGraph( CNodeView *p )
{
	pGraph = p;
	p->InvalidateLayout( true );
}

void CFlowGraphPage::PageShow()
{
	Assert( pGraph );
	pGraph->UploadPreviewData();
}
