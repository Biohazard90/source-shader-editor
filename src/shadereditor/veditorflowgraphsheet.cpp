
#include "cbase.h"
#include "editorcommon.h"


CFlowGraphSheet::CFlowGraphSheet( Panel *parent, const char *panelName, bool draggableTabs, bool closeableTabs )
	: BaseClass( parent, panelName, draggableTabs, closeableTabs )
{
	SetTabPosition( 1 );
}

CFlowGraphSheet::~CFlowGraphSheet()
{
}


void CFlowGraphSheet::PerformLayout()
{
	BaseClass::PerformLayout();

	int viewer_inset = 22;
	int viewer_inset_y = 45;
	int w,t;
	//engine->GetScreenSize( w, t );
	GetParent()->GetSize( w, t );
	SetPos( viewer_inset, viewer_inset_y );
	SetSize( w - viewer_inset * 2, t - viewer_inset - viewer_inset_y );
}
