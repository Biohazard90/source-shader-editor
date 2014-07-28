#ifndef CSHEET_TEXSAMPLE_H
#define CSHEET_TEXSAMPLE_H

#include "vSheets.h"


class CSheet_TextureSample : public CSheet_Base
{
public:
	DECLARE_CLASS_SIMPLE( CSheet_TextureSample, CSheet_Base );

	CSheet_TextureSample(CBaseNode *pNode, CNodeView *view, KeyValues *data, Panel *parent );
	~CSheet_TextureSample();

	virtual const char *GetSheetTitle(){return"Texture Sample";};

	MESSAGE_FUNC( OnResetData, "ResetData" );
	MESSAGE_FUNC( OnApplyChanges, "ApplyChanges" );

	MESSAGE_FUNC_CHARPTR( OnCommand, "Command", command );

	MESSAGE_FUNC_PARAMS( OnFileSelected, "FileSelected", fullpath );
	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );
	MESSAGE_FUNC_PARAMS( OnMenuClose, "OnMenuClose", pKV );
	
	virtual void Paint();

	void MakeSamplerOnly();

protected:

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	bool m_bSamplerOnly;

	bool bFileIsEnvmap;
	//static int m_iVguiPaint_Image;
	int m_iVguiPaint_Texture;
	void LoadFile();

	DHANDLE< FileOpenDialog >	m_hOpenTexture;
	char m_szDemoTexturePath[ MAX_PATH ];

	ComboBox *m_pComboBox_TextureType;
	TextEntry *m_pParameterName;

	CheckButton *m_pCheckSRGB_Read;
	TextEntry *m_pTextEntry_DemoTexture;
	Button *m_pButton_LoadTexture;


	TextEntry *m_pTextEntry_FallbackTexture;
	ComboBox *m_pComboBox_FallbackType;

	ComboBox *m_pComboBox_LookupOverride;


	void OnTexturetypeSelect();
	ITexture *m_pTexture;
	IMaterial *m_pMat_Tex;
	KeyValues *pKV_Mat;

	bool m_bReady;
};


#endif