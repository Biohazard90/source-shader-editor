#ifndef V_SMART_AUTO_COMPLETE_H
#define V_SMART_AUTO_COMPLETE_H

#include "cbase.h"
#include "vgui_controls/controls.h"
#include "vgui_controls/panel.h"
#include "vgui_controls/menu.h"

using namespace vgui;

class CSmartObjectList;
class CSmartTooltip;

class CSmartAutocomplete : public Menu
{
	DECLARE_CLASS_SIMPLE( CSmartAutocomplete, Menu );

public:
	CSmartAutocomplete( Panel *parent, const char *pElementname );
	~CSmartAutocomplete();

	// global identifier
	// if pszVarName != NULL then find as var, generate sub list with children
	bool InitList( CSmartObjectList *list, int textPos, int iScopeDepth, int iScopeNum,
		const char *pSourceFile, const char *pszVarName, bool bSort = true );

	// using database indices
	int FindEntry( const char *pWord );
	void ActivateEntry( int index );

	void PerformLayout();
	void OnKillFocus();

	void OnCursorEnteredMenuItem( int i ){};
	void OnCursorExitedMenuItem( int i ){};
	void OnMousePressed( MouseCode c ){};
	//static const int GetACSizeX(){ return 160; };
	//static const int GetACSizeY(){ return 80; };

	MESSAGE_FUNC_INT( InternalMousePressed, "MousePressed", code );
	MESSAGE_FUNC_INT( MenuItemHighlight, "MenuItemHighlight", itemID );
	
	void MatchInput( const char *str );
	void MatchInput( const wchar_t *str );
	int IsExactMatch( const char *str, bool bPartial );
	int IsExactMatch( const wchar_t *str, bool bPartial );

	char *AllocHighlightItemName();

	void SelectItem( int i );

private:

	void UpdateTooltip();

	int MatchSetVisible( bool bAll, const char *str );

	CSmartObjectList *m_pList;
	CSmartTooltip *m_pInfoLabel;

	int m_iImg_Var;
	int m_iImg_Func;
	int m_iImg_Struct;
	int m_iImg_Def;

	char *m_pMatchStr;
};


#endif