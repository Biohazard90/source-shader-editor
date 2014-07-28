#ifndef C_EDITOR_FG_SHEET_H
#define C_EDITOR_FG_SHEET_H

#include "vSheets.h"

#include "vgui_controls/Controls.h"
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/propertysheet.h>
#include <vgui_controls/propertypage.h>
#include <vgui_controls/textentry.h>
#include <vgui_controls/checkbutton.h>
#include <vgui_controls/slider.h>
#include <vgui_controls/combobox.h>
#include <vgui_controls/PanelListPanel.h>

class CFlowGraphSheet : public PropertySheet
{
public:
	DECLARE_CLASS_SIMPLE( CFlowGraphSheet, PropertySheet );

	CFlowGraphSheet( Panel *parent, const char *panelName, bool draggableTabs, bool closeableTabs );
	~CFlowGraphSheet();

protected:
   virtual void PerformLayout();

};


#endif