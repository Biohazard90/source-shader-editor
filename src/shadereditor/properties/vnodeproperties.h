#ifndef CNODEPROPERTIES_H
#define CNODEPROPERTIES_H

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

class CNodePropertySheet : public PropertyDialog
{
public:
	DECLARE_CLASS_SIMPLE( CNodePropertySheet, PropertyDialog );

	CNodePropertySheet(CBaseNode *pNode, CNodeView *parent, const char *panelName);
	~CNodePropertySheet();

	CBaseNode *GetNode();

	virtual void ResetAllData();

	KeyValues *GetPropertyContainer();

protected:
	virtual bool OnOK(bool applyOnly);
	virtual void OnCancel();

	CBaseNode *n;
	KeyValues *pKV_NodeSettings;
	KeyValues *pKV_NodeSettings_Original;

	CNodeView *pNodeView;

private:
	CNodePropertySheet( const CNodePropertySheet &other );
};


#endif