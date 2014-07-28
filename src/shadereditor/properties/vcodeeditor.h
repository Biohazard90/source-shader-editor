#ifndef CCODE_EDITOR_H
#define CCODE_EDITOR_H

#include "vSheets.h"

#include <vgui_controls/RichText.h>
#include "vSmartText.h"

//class CSmartText;
class CSheet_Base;
struct _smartRow;
struct text_cursor;

class CCode_Editor : public Frame
{
public:
	DECLARE_CLASS_SIMPLE( CCode_Editor, Frame );

	CCode_Editor( CSheet_Base *parent, const char *pName, CSmartText::CodeEditMode_t mode = CSmartText::CODEEDITMODE_HLSL );
	~CCode_Editor();

	void InitHlsl( const char *pszFunctionName = NULL, CUtlVector< __funcParamSetup* > *input = NULL, CUtlVector< __funcParamSetup* > *output = NULL,
		const char *pszCodeGlobal = NULL, const char *pszCodeBody = NULL,
		const int iHlslHierachy = HLSLHIERACHY_PS | HLSLHIERACHY_VS, const int iEnvData = 0 );
	void InitVMT( const char *pszVmt );

	void OnKeyCodeTyped( KeyCode code );
	void OnKeyTyped( wchar_t c );

	void OnCommand( const char *cmd );

	MESSAGE_FUNC( OnCodeChanged, "CodeChanged" );

private:

	CSmartText::CodeEditMode_t m_iMode;

	void ApplyCode( bool bDoInject );

	HHISTORYIDX m_iLastAppliedCodeIndex;

	void UpdateButtonEnabled( bool bShowSave, bool bShowApply );

	CSmartText *m_pCodeWindow;

	char *ReadSnippet( text_cursor c0, text_cursor c1, bool bAddCR = true );

};


#endif