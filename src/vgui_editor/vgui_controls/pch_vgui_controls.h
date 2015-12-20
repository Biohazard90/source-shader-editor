//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef PCH_VGUI_CONTROLS_H
#define PCH_VGUI_CONTROLS_H

#ifdef _WIN32
#pragma once
#endif

// general includes
#include <ctype.h>
#include <stdlib.h>
#include "tier0/dbg.h"
#include "tier0/valve_off.h"
#include "tier1/KeyValues.h"

#include "tier0/valve_on.h"
#include "tier0/memdbgon.h"

#include "filesystem.h"
#include "tier0/validator.h"

// vgui includes
#include "vgui/IBorder.h"
#include "vgui/IInput.h"
#include "vgui/ILocalize.h"
#include "vgui/IPanel.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"
#include "vgui/KeyCode.h"
#include "vgui/Cursor.h"
#include "vgui/MouseCode.h"

// vgui controls includes
#include "vgui_controls/controls.h"

#include "vgui_controls/animatingimagepanel.h"
#include "vgui_controls/animationcontroller.h"
#include "vgui_controls/bitmapimagepanel.h"
#include "vgui_controls/buildgroup.h"
#include "vgui_controls/buildmodedialog.h"
#include "vgui_controls/button.h"
#include "vgui_controls/checkbutton.h"
#include "vgui_controls/checkbuttonlist.h"
#include "vgui_controls/combobox.h"
#include "vgui_controls/controls.h"
#include "vgui_controls/dialogmanager.h"
#include "vgui_controls/directoryselectdialog.h"
#include "vgui_controls/divider.h"
#include "vgui_controls/editablepanel.h"
#include "vgui_controls/fileopendialog.h"
#include "vgui_controls/focusnavgroup.h"
#include "vgui_controls/frame.h"
#include "vgui_controls/graphpanel.h"
#include "vgui_controls/html.h"
#include "vgui_controls/image.h"
#include "vgui_controls/imagelist.h"
#include "vgui_controls/imagepanel.h"
#include "vgui_controls/label.h"
#include "vgui_controls/listpanel.h"
#include "vgui_controls/listviewpanel.h"
#include "vgui_controls/menu.h"
#include "vgui_controls/menubar.h"
#include "vgui_controls/menubutton.h"
#include "vgui_controls/menuitem.h"
#include "vgui_controls/messagebox.h"
#include "vgui_controls/panel.h"
#ifndef HL1
#include "vgui_controls/panelanimationvar.h"
#endif
#include "vgui_controls/panellistpanel.h"
#include "vgui_controls/phandle.h"
#include "vgui_controls/progressbar.h"
#include "vgui_controls/progressbox.h"
#include "vgui_controls/propertydialog.h"
#include "vgui_controls/propertypage.h"
#include "vgui_controls/propertysheet.h"
#include "vgui_controls/querybox.h"
#include "vgui_controls/radiobutton.h"
#include "vgui_controls/richtext.h"
#include "vgui_controls/scrollbar.h"
#include "vgui_controls/scrollbarslider.h"
#include "vgui_controls/sectionedlistpanel.h"
#include "vgui_controls/slider.h"
#ifndef HL1
#include "vgui_controls/splitter.h"
#endif
#include "vgui_controls/textentry.h"
#include "vgui_controls/textimage.h"
#include "vgui_controls/togglebutton.h"
#include "vgui_controls/tooltip.h"
#ifndef HL1
#include "vgui_controls/toolwindow.h"
#endif
#include "vgui_controls/treeview.h"
#ifndef HL1
#include "vgui_controls/treeviewlistcontrol.h"
#endif
#include "vgui_controls/urllabel.h"
#include "vgui_controls/wizardpanel.h"
#include "vgui_controls/wizardsubpanel.h"

#ifndef HL1
#include "vgui_controls/keyboardeditordialog.h"
#include "vgui_controls/inputdialog.h"
#endif

#endif // PCH_VGUI_CONTROLS_H
