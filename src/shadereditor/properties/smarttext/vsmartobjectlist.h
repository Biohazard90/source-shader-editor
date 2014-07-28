#ifndef SMART_OBJECT_LIST_H
#define SMART_OBJECT_LIST_H

#include "cbase.h"

#define IDENT_LIVE_CODE "__EDITORCODE"
#define IDENT_STATIC_LIVE_CODE "__EDITORCODE_AUTO"

class CSmartObject;

class CSmartObjectList
{
public:

	CSmartObjectList();
	~CSmartObjectList();
	CSmartObjectList( const CSmartObjectList &o );

	int GetNumEntries();
	CSmartObject *GetEntry( int i );
	CSmartObject *GetEntryByIdent( const char *pIdent );
	CSmartObject *GetEntryByIdent( const wchar_t *pIdent );

	void ParseCode( const char *pszFilename, bool bRecursive = false, CUtlVector< char* > *hIncludes = NULL );
	void ParseCode_Stream( CUtlBuffer &hStream, bool bRecursive = false, CUtlVector< char* > *hIncludes = NULL, const char *sourceFile = NULL, int scopeOffset = -1 );

	void DoSort();

	void MergeWithEntries( CSmartObjectList *pOther );
	void PurgeUnitsOfSameSource( const char *pszSrc );

	void AddUnit( CSmartObject *pUnit );

	void SetDatabaseCopy( CSmartObjectList *pCopyTarget );

	void BuildIntrinsicDatatypes();

private:

	void BuildHelpTexts( char *pStreamCopy, int iSize );

	void ParseCode_PreProc( CUtlVector< char* > &hLines, bool bRecursive, CUtlVector< char* > *hIncludes, const char *pTextStart );
	void ParseCode_KillPreProc( CUtlVector< char* > &hLines );
	void ParseCode_RemoveComments( CUtlBuffer &hStream );
	//void ParseCode_RemoveInvisibleExpressions( CUtlVector< char* > &hExpressions, int scopeOffset, const char *pTextStart, const int iSize );
	void ParseCode_Expressions( CUtlVector< char* > &hExpressions, const char *pTextStart );

	void Parse_Include( char *pRead, CUtlVector< char* > *hIncludes );
	void Parse_Define( char *pRead, const int iOffset );
	void Parse_Variable( char *pRead, const int iOffset, const int iScopeDepth, const int iScopeNum );
	void Parse_Function( char *pRead, const int iOffset, const int iScopeDepth, const int iScopeNum,
		const int iInlineScopeDepth, const int iInlineScopeNum );
	void Parse_Struct( char *pRead, const int iOffset, const int iScopeDepth, const int iScopeNum );


	bool DatatypeMatch( const char *pWord );

	CUtlVector< CSmartObject* >hAutoCompUnits;

	const char *m_pCurSourceFile;
	CSmartObjectList *m_pCopyTarget;
};



#endif