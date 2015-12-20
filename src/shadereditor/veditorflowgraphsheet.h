#ifndef C_EDITOR_FG_SHEET_H
#define C_EDITOR_FG_SHEET_H

#include "vsheets.h"

#include "vgui_controls/controls.h"
#include <vgui_controls/propertydialog.h>
#include <vgui_controls/propertysheet.h>
#include <vgui_controls/propertypage.h>
#include <vgui_controls/textentry.h>
#include <vgui_controls/checkbutton.h>
#include <vgui_controls/slider.h>
#include <vgui_controls/combobox.h>
#include <vgui_controls/panellistpanel.h>

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