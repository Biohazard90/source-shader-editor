#ifndef CHLSL_SHADERPRECACHE_H
#define CHLSL_SHADERPRECACHE_H

#include "cbase.h"
#include "editorCommon.h"

using namespace vgui;

struct PListEntry_t
{
	Label *L;
	Button *B;

	static int FindEntry( const CUtlVector< PListEntry_t > &list, const Label* L )
	{
		for ( int i = 0; i < list.Count(); i++ )
			if ( list[i].L == L )
				return i;
		return -1;
	}
	static int FindEntry( const CUtlVector< PListEntry_t > &list, const Button* B )
	{
		for ( int i = 0; i < list.Count(); i++ )
			if ( list[i].B == B )
				return i;
		return -1;
	}
	static bool HasElement( const CUtlVector< PListEntry_t > &list, const Label* L )
	{
		return FindEntry( list, L ) >= 0;
	}
	static bool HasElement( const CUtlVector< PListEntry_t > &list, const Button* B )
	{
		return FindEntry( list, B ) >= 0;
	}
};

class CShaderPrecache : public CBaseDiag
{
	DECLARE_CLASS_SIMPLE( CShaderPrecache, CBaseDiag );

public:
	CShaderPrecache( vgui::Panel *parent, CNodeView *nodeview );
	~CShaderPrecache();

	virtual void Activate();

	void AddEntry( const char *name );
	void LoadList();

	void SetButtonCmd( Button *b );
	void UpdateAllButtonCmds();
	void SaveList();

	MESSAGE_FUNC_PARAMS( OnFileSelected, "FileSelected", fullpath );

protected:
	virtual void Paint();

	void OpenFileBrowser();

	void OnCommand(const char* pcCommand);

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();

	MESSAGE_FUNC_INT( OnRemovePressed, "shaderremove", index );

private:
	DHANDLE< FileOpenDialog >	m_hDumpBrowser;

	PanelListPanel *m_pPanelList_Shader;

	CUtlVector< PListEntry_t >m_pPanelList;
	void SortAllEntries();

	CheckButton *m_pCheck_ReloadCache;
};

#endif