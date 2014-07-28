
#include "cbase.h"
#include "cRegex.h"
#include "vSmartObject.h"
#include "vSmartObjectList.h"

#include "editorCommon.h"

#define __DBG_PARSE DEBUG

#if __DBG_PARSE
void ParseDbg( const char *msg, ... )
{
	va_list argptr;
	char str[512];
	Q_memset( str, 0, sizeof( str ) );

	va_start( argptr, msg );
	Q_vsnprintf( str, sizeof( str ), msg, argptr );
	va_end( argptr );

	Msg( "parser: %s\n", str );
}
#else
#define ParseDbg( ... ) ((void)NULL)
#endif

int NextNull( char **p )
{
	int count = 0;

	while ( p && *p && **p )
	{
		(*p)++;
		count++;
	}

	return count;
}
int NextLine( char **p )
{
	int count = 0;

	while ( p && *p && **p )
	{
		bool bLF = **p == '\n';

		(*p)++;
		count++;

		if ( bLF )
			break;
	}

	return count;
}
void PreviousLine( char **p, int &size )
{
	bool bGotStartThisLine = false;
	while ( size > 0 )
	{
		bool bLineStart = **p == '\n';

		if ( bLineStart )
		{
			if ( bGotStartThisLine )
			{
				(*p)++;
				size++;
				break;
			}
			else
				bGotStartThisLine = true;
		}

		(*p)--;
		size--;
	}
}
int NextCharacter( char **p )
{
	int count = 0;

	while ( p && *p && **p )
	{
		if ( !IS_SPACE( **p ) )
			break;

		(*p)++;
		count++;
	}

	return count;
}
int NextSpace( char **p )
{
	int count = 0;

	while ( p && *p && **p )
	{
		if ( IS_SPACE( **p ) )
			break;

		(*p)++;
		count++;
	}

	return count;
}
// WORD
// |   
char *MoveToWordLeft( char *pStrStart, char *pWord )
{
	Assert( pStrStart && pWord && pWord >= pStrStart );

	while ( pWord >= pStrStart &&
		( IS_CHAR( *pWord ) || IS_NUMERICAL( *pWord ) ) && !IS_SPACE( *pWord ) )
	{
		pWord--;
	}

	pWord++;

	return pWord;
}
// WORD
//     |
char *MoveToWordRight( char *pWord )
{
	Assert( *pWord );

	while ( *pWord &&
		( IS_CHAR( *pWord ) || IS_NUMERICAL( *pWord ) ) && !IS_SPACE( *pWord ) )
	{
		pWord++;
	}

	//pWord--;

	return pWord;
}
char *CopyWord( char *pStrStart, int wordPos = 0 )
{
	char *pWord = pStrStart;

	while ( pWord && *pWord && wordPos > 0 )
	{
		pWord++;
		wordPos--;
	}

	Assert( pWord && *pWord && !IS_SPACE(*pWord) );

	if ( !*pWord )
		return NULL;

	if ( IS_SPACE( *pWord ) )
		return NULL;

	char *pWordStart = MoveToWordLeft( pStrStart, pWord );
	char *pWordEnd = MoveToWordRight( pWord );

	int wSize = pWordEnd - pWordStart;

	if ( wSize < 1 )
		return NULL;

	char *pOut = new char[ wSize + 1 ];
	Q_memcpy( pOut, pWordStart, wSize );
	pOut[ wSize ] = '\0';

	return pOut;
}
char *CopyToDelimiter( char *pStart, const char delim = '\n', bool bInclusive = false )
{
	char *pOut = NULL;
	char *pDelimitedStart = pStart;
	char *pDelimitedEnd = pDelimitedStart;

	while ( *pDelimitedEnd && *pDelimitedEnd != delim )
		pDelimitedEnd++;

	if ( *pDelimitedEnd == delim && bInclusive )
		pDelimitedEnd++;

	int delimSize = pDelimitedEnd - pDelimitedStart;
	if ( delimSize > 0 )
	{
		pOut = new char[ delimSize + 1 ];
		Q_memcpy( pOut, pDelimitedStart, delimSize );
		pOut[ delimSize ] = '\0';
	}

	return pOut;
}
void ConvertCharacter( char *pStr, char from, char to )
{
	while ( pStr && *pStr )
	{
		if ( *pStr == from )
			*pStr = to;
		pStr++;
	}
}
void CleanSpaces( char *pStr )
{
	bool bHadSpace = false;
	char *pSearch = pStr;
	int len = Q_strlen( pStr );

	while ( pSearch && *pSearch )
	{
		if ( IS_SPACE( *pSearch ) )
		{
			if ( *pSearch == '\t' || *pSearch == '\r' )
			{
				*pSearch = ' ';
			}

			if ( bHadSpace || pSearch == pStr )
			{
				*pSearch = '\0';

				char *pRest = pSearch + 1;

				if ( *pRest )
				{
					Assert( Q_strlen( pStr ) < len );
					Q_strcat( pStr, pRest, len );
					continue;
				}
			}

			bHadSpace = true;
		}
		else
			bHadSpace = false;

		pSearch++;
	}
}

CSmartObjectList::CSmartObjectList()
{
	m_pCurSourceFile = NULL;
	m_pCopyTarget = NULL;
}
CSmartObjectList::~CSmartObjectList()
{
	hAutoCompUnits.PurgeAndDeleteElements();
}
CSmartObjectList::CSmartObjectList( const CSmartObjectList &o )
{
	m_pCurSourceFile = NULL;
	m_pCopyTarget = NULL;

	for ( int i = 0; i < o.hAutoCompUnits.Count(); i++ )
		hAutoCompUnits.AddToTail( new CSmartObject( *o.hAutoCompUnits[i] ) );

	Assert( hAutoCompUnits.Count() == o.hAutoCompUnits.Count() );
}

int ACompSort( CSmartObject * const *p1, CSmartObject * const *p2 )
{
	if ( !(*p1)->m_pszIdentifier || !(*p2)->m_pszIdentifier )
		return 0;

	return Q_stricmp( (*p1)->m_pszIdentifier, (*p2)->m_pszIdentifier );
}
void CSmartObjectList::DoSort()
{
	hAutoCompUnits.Sort( ACompSort );
}

int CSmartObjectList::GetNumEntries()
{
	return hAutoCompUnits.Count();
}
CSmartObject *CSmartObjectList::GetEntry( int i )
{
	Assert( i >= 0 && i < GetNumEntries() );

	return hAutoCompUnits[ i ];
}
CSmartObject *CSmartObjectList::GetEntryByIdent( const char *pIdent )
{
	for ( int i = 0; i < GetNumEntries(); i++ )
		if ( GetEntry(i)->m_pszIdentifier && !Q_strcmp( GetEntry(i)->m_pszIdentifier, pIdent ) )
			return GetEntry(i);
	return NULL;
}
CSmartObject *CSmartObjectList::GetEntryByIdent( const wchar_t *pIdent )
{
	if ( !pIdent || !Q_wcslen( pIdent ) )
		return NULL;

	int len = Q_wcslen( pIdent ) + 1;
	char *pCStr = new char[ len ];
	Q_UnicodeToUTF8( pIdent, pCStr, len );

	CSmartObject *pOut = GetEntryByIdent( pCStr );

	delete [] pCStr;
	return pOut;
}

bool CSmartObjectList::DatatypeMatch( const char *pWord )
{
	if ( CRegex::MatchSyntax_Datatypes( pWord ) != __REGEX_INVALID )
		return true;

	for ( int s = 0; s < hAutoCompUnits.Count(); s++ )
		if ( hAutoCompUnits[s]->m_iType == ACOMP_STRUCT_DEF &&
			hAutoCompUnits[s]->m_pszIdentifier != NULL &&
			!Q_stricmp( pWord, hAutoCompUnits[s]->m_pszIdentifier ) )
			return true;

	return false;
}

void CSmartObjectList::AddUnit( CSmartObject *pUnit )
{
	//delete pUnit;

	bool bDoAdd = true;

	if ( pUnit->m_pszIdentifier && pUnit->m_iType != ACOMP_INVALID )
	{
		for ( int i = 0; i < hAutoCompUnits.Count(); i++ )
		{
			CSmartObject *p = hAutoCompUnits[i];

			// no name, ignore
			if ( !p->m_pszIdentifier )
				continue;

			// different name, ignore
			if ( Q_stricmp( p->m_pszIdentifier, pUnit->m_pszIdentifier ) )
				continue;

			if ( p->IsObjectVisible( pUnit ) )
				continue;

			if ( p->m_iScopeDepth != pUnit->m_iScopeDepth ||
				p->m_iScopeNum != pUnit->m_iScopeNum )
				continue;

			// same name + different type -> delete
			if ( p->m_iType != pUnit->m_iType )
				bDoAdd = false;

			//overloads for functions
			if ( pUnit->m_iType == ACOMP_FUNC )
			{
				// same amount of children, test them; different amount == valid overload
				if ( pUnit->m_hChildren.Count() == p->m_hChildren.Count() )
				{
					bool bDifferentChild = false;
					for ( int c = 0; c < pUnit->m_hChildren.Count(); c++ )
					{
						CSmartObject *c0 = pUnit->m_hChildren[ c ];
						CSmartObject *c1 = p->m_hChildren[ c ];

						if ( *c0 != *c1 )
							bDifferentChild = true;
					}
					if ( !bDifferentChild )
						bDoAdd = false;
				}
			}
			else
				bDoAdd = false;
		}
	}
	else
		bDoAdd = false;

	//Assert( bDoAdd );

	if ( bDoAdd )
	{
		if ( m_pCurSourceFile != NULL && *m_pCurSourceFile && pUnit->m_pszSourceFile == NULL )
			AutoCopyStringPtr( m_pCurSourceFile, &pUnit->m_pszSourceFile );

		hAutoCompUnits.AddToTail( pUnit );

		if ( m_pCopyTarget )
			m_pCopyTarget->hAutoCompUnits.AddToTail( new CSmartObject( *pUnit ) );
		//pUnit->DebugPrint();
	}
	else
		delete pUnit;
}

void CSmartObjectList::SetDatabaseCopy( CSmartObjectList *pCopyTarget )
{
	m_pCopyTarget = pCopyTarget;
}

void CSmartObjectList::MergeWithEntries( CSmartObjectList *pOther )
{
	CUtlVector< char* >hSources;

	for ( int i = 0; i < pOther->GetNumEntries(); i++ )
	{
		CSmartObject *pO = pOther->GetEntry( i );

		if ( !pO->m_pszSourceFile )
			continue;

		bool bFound = false;

		for ( int c = 0; c < hSources.Count(); c++ )
		{
			if ( !Q_stricmp( hSources[c], pO->m_pszSourceFile ) )
				bFound = true;
		}

		if ( !bFound )
			hSources.AddToTail( pO->m_pszSourceFile );
	}

	for ( int c = 0; c < hSources.Count(); c++ )
		PurgeUnitsOfSameSource( hSources[c] );

	hSources.Purge();

	for ( int i = 0; i < pOther->GetNumEntries(); i++ )
		AddUnit( new CSmartObject( *pOther->GetEntry( i ) ) );
}

void CSmartObjectList::ParseCode( const char *pszFilename, bool bRecursive, CUtlVector< char* > *hIncludes )
{
	if ( hIncludes != NULL )
	{
		for ( int i = 0; i < hIncludes->Count(); i++ )
			if ( !Q_stricmp( hIncludes->Element( i ), pszFilename ) )
				return;
	}

	char cFile[MAXTARGC];
	Q_snprintf( cFile, sizeof( cFile ), "%s\\%s", GetShaderSourceDirectory(), pszFilename );

	bool bExists = g_pFullFileSystem->FileExists( cFile );
	//Assert( bExists );

	//for ( int i = 0; i < 2000; i++ )
	//	bExists = g_pFullFileSystem->FileExists( cFile );

	if ( !bExists )
		return;

	int iFSize = g_pFullFileSystem->Size( cFile );
	Assert( iFSize > 0 );
	iFSize++;

	if ( iFSize <= 0 )
		return;

#if __DBG_PARSE
	Warning( "trying to alloc %i bytes... (step 1)\n", iFSize );
#endif

	char *pBuff = new char[ iFSize ];
	Assert( pBuff != NULL );

	if ( pBuff == NULL )
		return;

	Q_memset( pBuff, 0, iFSize );

	

	char *pCopyBuff = new char[ iFSize ];

#if __DBG_PARSE
	Warning( "trying to alloc %i bytes... (step 2)\n", iFSize );
#endif

	CUtlBuffer buf( pBuff, iFSize );
	if ( pCopyBuff && g_pFullFileSystem->ReadFile( cFile, NULL, buf ) )
	{
		char *pTextClearCR = pBuff;
		int iSize = buf.Size();

		while ( iSize > 0 )
		{
			if ( *pTextClearCR == '\r' )
				*pTextClearCR = ' ';

			pTextClearCR++;
			iSize--;
		}

		Q_memcpy( pCopyBuff, pBuff, iFSize );

		CUtlVector< CSmartObject* >hOldUnits;
		hOldUnits.AddVectorToTail( hAutoCompUnits );
		hAutoCompUnits.Purge();

		ParseCode_Stream( buf, bRecursive, hIncludes, pszFilename );

		BuildHelpTexts( pCopyBuff, buf.Size() );

		CUtlVector< CSmartObject* >hNewUnits;
		hNewUnits.AddVectorToTail( hAutoCompUnits );
		hAutoCompUnits.Purge();

		hAutoCompUnits.AddVectorToTail( hOldUnits );
		for ( int i = 0; i < hNewUnits.Count(); i++ )
			AddUnit( hNewUnits[ i ] );

		hOldUnits.Purge();
		hNewUnits.Purge();
	}

	buf.Purge();
	delete [] pBuff;
	delete [] pCopyBuff;
}

void CSmartObjectList::BuildHelpTexts( char *pStreamCopy, int iSize )
{
	pStreamCopy[ iSize - 1 ] = '\0';

	for ( int i = 0; i < hAutoCompUnits.Count(); i++ )
	{
		CSmartObject *p = hAutoCompUnits[i];

		if ( p->m_bHelpLocked )
			continue;

		p->m_bHelpLocked = true;

		int offset = p->m_iTextOffset;

		if ( offset < 0 )
			continue;

		Assert( offset >= 0 && offset < iSize );

		bool bLookRight = p->m_iType == ACOMP_DEFINE || p->m_iType == ACOMP_VAR;
		char *pLineStart = pStreamCopy + offset;

		char *pSearchCommentStart = pLineStart;
		int searchSizeStart = offset;

		CUtlVector< char* >hHelpLines;

		if ( bLookRight )
		{
			while ( searchSizeStart < iSize - 2 && *pSearchCommentStart != '\n' )
			{
				if ( *pSearchCommentStart == '/' && *(pSearchCommentStart+1) == '/' )
				{
					hHelpLines.AddToTail( pSearchCommentStart );
					break;
				}

				pSearchCommentStart++;
				searchSizeStart++;
			}
		}

		if ( !hHelpLines.Count() )
		{
			pSearchCommentStart = pLineStart - 1;
			searchSizeStart = offset;

			while ( searchSizeStart >= 0 && ( IS_SPACE( *pSearchCommentStart ) || *pSearchCommentStart == '\n' ) )
			{
				pSearchCommentStart--;
				searchSizeStart--;
			}

			// multiline comment, need to search from the very beginning
			if ( searchSizeStart > 1 && *pSearchCommentStart == '/' && *(pSearchCommentStart-1) == '*' )
			{
				bool bInComment = false;
				char *pSearchMCStart = pStreamCopy;
				char *pLastMCStart = NULL;
				char *pLastMCEnd = NULL;

				while ( pSearchMCStart <= pSearchCommentStart )
				{
					if ( *pSearchMCStart == '/' && *(pSearchMCStart+1) == '/' )
					{
						NextLine( &pSearchMCStart );
						continue;
					}
					else if ( !bInComment && *pSearchMCStart == '/' && *(pSearchMCStart+1) == '*' )
					{
						pLastMCStart = pSearchMCStart;
						bInComment = true;
					}
					else if ( bInComment && *(pSearchMCStart-1) == '*' && *(pSearchMCStart) == '/' )
					{
						pLastMCEnd = pSearchMCStart;
						bInComment = false;
					}

					pSearchMCStart++;
				}

				if ( !bInComment && pLastMCEnd == pSearchCommentStart )
				{
					char *pListCmtLines = pLastMCStart;
					while ( pListCmtLines < pLastMCEnd )
					{
						hHelpLines.AddToTail( pListCmtLines );
						NextLine( &pListCmtLines );
					}
				}
			}
			// check for single line comment and extent upwards for as many lines as possible
			else
			{
				pSearchCommentStart = pLineStart;
				searchSizeStart = offset;

				for (;;)
				{
					PreviousLine( &pSearchCommentStart, searchSizeStart );

					if ( *pSearchCommentStart == '/' && *(pSearchCommentStart+1) == '/' )
						hHelpLines.AddToHead( pSearchCommentStart );
					else break;

					if ( searchSizeStart <= 0 )
						break;
				}
			}
		}

		if ( hHelpLines.Count() )
		{
			CUtlVector< char* >hCleanLines;

			for ( int i = 0; i < hHelpLines.Count(); i++ )
			{
				bool bAddLinebreak = i < hHelpLines.Count() - 1;
				hCleanLines.AddToTail(CopyToDelimiter( hHelpLines[i], '\n', bAddLinebreak ));
			}

			int completeLength = 1;
			for ( int i = 0; i < hCleanLines.Count(); i++ )
			{
				if ( hCleanLines[ i ] == NULL )
				{
					hCleanLines.Remove( i );
					i--;
				}

				completeLength += Q_strlen( hCleanLines[ i ] );
			}

			char *pCombined = new char[ completeLength ];
			pCombined[ 0 ] = '\0';

			for ( int i = 0; i < hCleanLines.Count(); i++ )
			{
				Q_strcat( pCombined, hCleanLines[ i ], completeLength );
				delete [] hCleanLines[ i ];
			}

			hCleanLines.Purge();

			p->m_pszHelptext = pCombined;
		}

		hHelpLines.Purge();
	}

	//for ( int i = 0; i < hAutoCompUnits.Count(); i++ )
	//	hAutoCompUnits[ i ]->DebugPrint();
}


void CSmartObjectList::Parse_Include( char *pRead, CUtlVector< char* > *hIncludes )
{
	Assert( hIncludes != NULL );

	char *pBracketStart = pRead;
	char *pBracketEnd = NULL;

	while ( pBracketStart && *pBracketStart )
	{
		if ( *pBracketStart == '\"' || *pBracketStart == '<' )
		{
			pBracketEnd = pBracketStart + 1;
			while ( *pBracketEnd )
			{
				if ( *pBracketEnd == *pBracketStart )
					break;
				pBracketEnd++;
			}
			break;
		}
		pBracketStart++;
	}

	if ( pBracketEnd && *pBracketEnd )
	{
		pBracketStart++;
		*pBracketEnd = '\0';
		ParseDbg( "valid include: %s -> %s", pRead, pBracketStart );

		ParseCode( pBracketStart, true, hIncludes );
	}
	else
	{
		ParseDbg( "invalid include: %s", pRead );
	}

	return;
}

void CSmartObjectList::Parse_Define( char *pRead, const int iOffset )
{
	const char *pszDef = "define";
	char *pSearch_Def = Q_stristr( pRead, pszDef );
	if ( !pRead )
		return;

	char *pExpStart_0 = pSearch_Def + Q_strlen( pszDef ) + 1;

	NextCharacter( &pExpStart_0 );

	if ( !*pExpStart_0 )
		return;

	char *pExpEnd = pExpStart_0;
	if ( !NextSpace( &pExpEnd ) || !IS_SPACE( *pExpEnd ) )
		return;

	//*pExpEnd = '\0';

	char *pExpStart_1 = pExpEnd;
	if ( !NextCharacter( &pExpStart_1 ) )
		return;

	if ( !*pExpStart_1 )
		return;

	int len = Q_strlen( pRead );
	char *pDef = new char[ len + 1 ];
	Q_memcpy( pDef, pRead, len );
	pDef[len] = '\0';
	char *pIdent = CopyWord( pExpStart_0 );
	CleanSpaces( pDef );

	if ( !DatatypeMatch( pIdent ) )
	{
		CSmartObject *p = new CSmartObject( ACOMP_DEFINE, pDef, pIdent );
		p->m_iTextOffset = iOffset;
		AddUnit( p );
	}

	delete [] pDef;
	delete [] pIdent;
}

void CSmartObjectList::Parse_Variable( char *pRead, const int iOffset, const int iScopeDepth, const int iScopeNum )
{
	int iSteps = 0;
	char *pDataType = NULL;
	char *pIdent = NULL;
	char *pVar_Search = pRead;

	for(;iSteps < 32;)
	{
		iSteps++;

		NextCharacter( &pVar_Search );

		char *pWord = CopyWord( pVar_Search );

		if ( !pWord )
		{
			Assert( 0 );
			break;
		}

		bool bSuccess = DatatypeMatch( pWord ) || pDataType != NULL;
		delete [] pWord;

		if ( bSuccess )
		{
			if ( pDataType != NULL )
			{
				pIdent = pVar_Search;
				break;
			}
			else
				pDataType = pVar_Search;
		}

		NextSpace( &pVar_Search );
	}

	Assert( iSteps < 31 && pDataType && pIdent );

	if ( !pDataType || !pIdent )
		return;

	pIdent = CopyWord( pIdent );
	pDataType = CopyWord( pDataType );
	CleanSpaces( pRead );

	if ( !DatatypeMatch( pIdent ) )
	{
		CSmartObject *p = new CSmartObject( ACOMP_VAR, pRead, pIdent, pDataType );
		p->m_iTextOffset = iOffset;
		p->m_iScopeDepth = iScopeDepth;
		p->m_iScopeNum = iScopeNum;
		AddUnit( p );
	}

	delete [] pIdent;
	delete [] pDataType;
}

void CSmartObjectList::Parse_Function( char *pRead, const int iOffset, const int iScopeDepth, const int iScopeNum,
	const int iInlineScopeDepth, const int iInlineScopeNum )
{
	int iSteps = 0;
	char *pDataType = NULL;
	char *pIdent = NULL;
	char *pVar_Search = pRead;

	char *pParamEnd = NULL;

	CUtlVector< CSmartObject* >hChildren;

	for(;iSteps < 32;)
	{
		iSteps++;

		NextCharacter( &pVar_Search );

		char *pWord = CopyWord( pVar_Search );

		if ( !pWord )
		{
			Assert( 0 );
			break;
		}

		bool bSuccess = DatatypeMatch( pWord ) || pDataType != NULL;
		delete [] pWord;

		if ( bSuccess )
		{
			if ( pDataType != NULL )
				pIdent = pVar_Search;
			else
				pDataType = pVar_Search;
		}

		if ( pIdent == NULL )
			NextSpace( &pVar_Search );
		else
		{
			char *pParamStart = pVar_Search;
			while ( *pParamStart && *pParamStart != '(' )
				pParamStart++;

			if ( !*pParamStart || !*(pParamStart+1) )
				return;
			pParamStart++;

			int bracketCount = 0;
			pParamEnd = pParamStart;
			// blah ( omg() )
			//       |      |
			//
			// adfsda ()
			//         |
			while ( *pParamEnd && ( *pParamEnd != ')' || bracketCount != 0 ) )
			{
				if ( *pParamEnd == '(' )
					bracketCount++;
				else if ( *pParamEnd == ')' )
					bracketCount--;
				pParamEnd++;
			}

			if ( pParamEnd <= pParamStart )
				break;

			char *pSearchVarStuff = pParamStart;
			char *pVar_Datatype = NULL;
			char *pVar_Name = NULL;
			const char *pParam_Start = NULL;

			while ( pSearchVarStuff < pParamEnd )
			{
				NextCharacter( &pSearchVarStuff );

				if ( !*pSearchVarStuff || pSearchVarStuff == pParamEnd )
					break;

				if ( !pParam_Start )
					pParam_Start = pSearchVarStuff;

				char *pWord = CopyWord( pSearchVarStuff );

				if ( !pWord )
					break;

				if ( pVar_Datatype != NULL )
				{
					pVar_Name = pWord;
					pWord = NULL;
				}
				else if ( DatatypeMatch( pWord ) )
				{
					pVar_Datatype = pWord;
					pWord = NULL;
				}

				if ( pVar_Name && pVar_Datatype )
				{
					char *pDef = NULL;
					if ( pParam_Start )
					{
						const char *pParam_End = pParam_Start;
						while ( *pParam_End && *pParam_End != ',' && *pParam_End != ')' )
							pParam_End++;

						while ( pParam_End > pParam_Start && IS_SPACE( *(pParam_End-1) ) )
							pParam_End--;

						int len = pParam_End - pParam_Start + 1;
						pDef = new char[ len ];
						Q_memcpy( pDef, pParam_Start, len - 1 );
						pDef[len-1] = '\0';

						pParam_Start = NULL;
					}
					CSmartObject *pC = new CSmartObject( ACOMP_VAR, pDef, pVar_Name, pVar_Datatype );
					delete [] pDef;
					delete [] pVar_Name;
					delete [] pVar_Datatype;
					pVar_Datatype = NULL;
					pVar_Name = NULL;

					hChildren.AddToTail( pC );

					while ( *pSearchVarStuff && *pSearchVarStuff != ',' )
						pSearchVarStuff++;

					if ( *pSearchVarStuff )
						pSearchVarStuff++;
				}
				else
					NextSpace( &pSearchVarStuff );

				delete [] pWord;
			}

			delete [] pVar_Datatype;
			delete [] pVar_Name;

			break;
		}
	}

	Assert( iSteps < 31 && pDataType && pIdent && pParamEnd );

	if ( !pDataType || !pIdent || !pParamEnd )
	{
		hChildren.PurgeAndDeleteElements();
		return;
	}

	int defSize = pParamEnd - pRead + 1;
	char *pDef = new char[ defSize + 1 ];
	Q_memcpy( pDef, pRead, defSize );
	pDef[ defSize ] = '\0';
	CleanSpaces( pDef );

	pIdent = CopyWord( pIdent );
	pDataType = CopyWord( pDataType );

	if ( !DatatypeMatch( pIdent ) )
	{
		CSmartObject *p = new CSmartObject( ACOMP_FUNC, pDef, pIdent, pDataType );
		p->m_iTextOffset = iOffset;
		p->m_iScopeDepth = iScopeDepth;
		p->m_iScopeNum = iScopeNum;
		p->m_hChildren.AddVectorToTail( hChildren );
		AddUnit( p );

		for ( int i = 0; i < hChildren.Count(); i++ )
		{
			CSmartObject *p = new CSmartObject( *hChildren[i] );
			p->m_iTextOffset = iOffset;
			p->m_iScopeDepth = iInlineScopeDepth;
			p->m_iScopeNum = iInlineScopeNum;
			AddUnit( p );
		}

		hChildren.Purge();
	}
	else
		hChildren.PurgeAndDeleteElements();

	delete [] pIdent;
	delete [] pDataType;
	delete [] pDef;
}
void CSmartObjectList::Parse_Struct( char *pRead, const int iOffset, const int iScopeDepth, const int iScopeNum )
{
	char *pDefStart = Q_stristr( pRead, "struct" );
	char *pStructStart = pDefStart;
	char *pIdent = NULL;

	if ( !pStructStart )
		return;

	NextSpace( &pStructStart );
	NextCharacter( &pStructStart );

	pIdent = pStructStart;

	char *pSearch_Children = pStructStart;
	while ( *pSearch_Children && *pSearch_Children != '{' )
		pSearch_Children++;

	if ( !*pSearch_Children )
		return;

	pSearch_Children++;
	NextCharacter( &pSearch_Children );

	char *pStructEnd = pSearch_Children;

	while ( *pStructEnd && *pStructEnd != '}' )
		pStructEnd++;

	if ( !*pStructEnd )
		return;

	CUtlVector< CSmartObject* >hChildren;

	//int iSteps = 0;

	{
		int structlen = pStructEnd - pSearch_Children + 1;
		char *localStruct = new char[ structlen + 1 ];
		Q_memcpy( localStruct, pSearch_Children, structlen );
		localStruct[ structlen ] = '\0';
		ConvertCharacter( localStruct, '\n', ' ' );
		pSearch_Children = localStruct;
		char *pStructEnd_Local = pSearch_Children + structlen - 1;

		char *pVar_Datatype = NULL;
		char *pVar_Name = NULL;
		char *pVar_Search = pSearch_Children;


		while ( pVar_Search < pStructEnd_Local )
		{
			NextCharacter( &pVar_Search );

			if ( !*pVar_Search || pVar_Search == pStructEnd_Local )
				break;

			char *pWord = CopyWord( pVar_Search );

			if ( !pWord )
				break;

			if ( pVar_Datatype != NULL )
			{
				pVar_Name = pWord;
				pWord = NULL;
			}
			else if ( DatatypeMatch( pWord ) )
			{
				pVar_Datatype = pWord;
				pWord = NULL;
			}

			if ( pVar_Name && pVar_Datatype )
			{
				CSmartObject *pC = new CSmartObject( ACOMP_VAR, NULL, pVar_Name, pVar_Datatype );
				delete [] pVar_Name;
				delete [] pVar_Datatype;
				pVar_Datatype = NULL;
				pVar_Name = NULL;

				hChildren.AddToTail( pC );

				while ( *pVar_Search && *pVar_Search != ';' )
					pVar_Search++;

				if ( *pVar_Search )
					pVar_Search++;
			}
			else
				NextSpace( &pVar_Search );

			delete [] pWord;
		}

		delete [] pVar_Datatype;
		delete [] pVar_Name;
		delete [] localStruct;
	}

	Assert( /*iSteps < 31 &&*/ pIdent && pStructEnd );


	if ( !pIdent || !pStructEnd )
	{
		hChildren.PurgeAndDeleteElements();
		return;
	}

	int defSize = pStructEnd - pDefStart + 1;
	char *pDef = new char[ defSize + 1 ];
	Q_memcpy( pDef, pDefStart, defSize );
	pDef[ defSize ] = '\0';
	CleanSpaces( pDef );

	pIdent = CopyWord( pIdent );

	CSmartObject *p = new CSmartObject( ACOMP_STRUCT_DEF, pDef, pIdent );
	p->m_iTextOffset = iOffset;
	p->m_iScopeDepth = iScopeDepth;
	p->m_iScopeNum = iScopeNum;
	p->m_hChildren.AddVectorToTail( hChildren );
	AddUnit( p );

	hChildren.Purge();
	delete [] pIdent;
	delete [] pDef;
}

void CSmartObjectList::ParseCode_Stream( CUtlBuffer &hStream, bool bRecursive, CUtlVector< char* > *hIncludes, const char *sourceFile, int scopeOffset )
{
	bool bOwnsIncludeVec = false;

	if ( bRecursive )
	{
		if ( !hIncludes )
		{
			bOwnsIncludeVec = true;
			hIncludes = new CUtlVector< char* >();
		}

		Assert( hIncludes );

		char *pSelf = NULL;
		AutoCopyStringPtr( sourceFile, &pSelf );

		hIncludes->AddToTail( pSelf );
	}

	const char *pszOldSrcFile = m_pCurSourceFile;
	m_pCurSourceFile = sourceFile;

	if ( m_pCurSourceFile != NULL && *m_pCurSourceFile )
		PurgeUnitsOfSameSource( m_pCurSourceFile );

	ParseCode_RemoveComments( hStream );

	char *pText = (char*)hStream.Base();
	char *pRead_LineStart = pText;

	// list lines
	CUtlVector< char* >hLines;
	const int Size = hStream.Size();
	int iSizeItr = Size;

	while ( pRead_LineStart && *pRead_LineStart && iSizeItr > 0 )
	{
		bool bHasChars = false;

		while ( *pRead_LineStart && iSizeItr > 0 )
		{
			if ( *pRead_LineStart == '\n' )
				break;

			else if ( !IS_SPACE( *pRead_LineStart ) )
			{
				bHasChars = true;
				break;
			}
			pRead_LineStart++;
			iSizeItr--;
		}

		if ( bHasChars )
			hLines.AddToTail( pRead_LineStart );

		int moved = NextLine( &pRead_LineStart );
		iSizeItr -= moved;
	}

	// per physical line
	ParseCode_PreProc( hLines, bRecursive, hIncludes, pText );

	// kill preprocessor code
	ParseCode_KillPreProc( hLines );

	// list expressions
	CUtlVector< char* >hExpressions;
	char *pReadExpression = pText;
	iSizeItr = Size;

	while ( pReadExpression && *pReadExpression && iSizeItr > 0 )
	{
		// first nonwhite space char
		while ( *pReadExpression && iSizeItr > 0 && IS_SPACE( *pReadExpression ) || *pReadExpression == '\n' )
		{
			pReadExpression++;
			iSizeItr--;
		}

		hExpressions.AddToTail( pReadExpression );

		// end of current expression
		while ( *pReadExpression && iSizeItr > 0 && *pReadExpression != ';' )
		{
			pReadExpression++;
			iSizeItr--;
		}

		if ( *pReadExpression == ';' && iSizeItr > 0 )
		{
			pReadExpression++;
			iSizeItr--;
		}
	}

	// code per expression
	//ParseCode_RemoveInvisibleExpressions( hExpressions, scopeOffset, pText, Size );
	ParseCode_Expressions( hExpressions, pText );

	hExpressions.Purge();
	hLines.Purge();

	m_pCurSourceFile = pszOldSrcFile;

	if ( bOwnsIncludeVec )
	{
		for ( int i = 0; i < hIncludes->Count(); i++ )
			delete [] hIncludes->Element( i );

		hIncludes->Purge();
		delete hIncludes;
	}
}
void CSmartObjectList::PurgeUnitsOfSameSource( const char *pszSrc )
{
	for ( int i = 0; i < hAutoCompUnits.Count(); i++ )
	{
		if ( !hAutoCompUnits[i]->m_pszSourceFile || !*hAutoCompUnits[i]->m_pszSourceFile )
			continue;

		if ( Q_stricmp( pszSrc, hAutoCompUnits[i]->m_pszSourceFile ) )
			continue;

		delete hAutoCompUnits[i];
		hAutoCompUnits.Remove(i);
		i--;
	}
}
void CSmartObjectList::ParseCode_PreProc( CUtlVector< char* > &hLines, bool bRecursive, CUtlVector< char* > *hIncludes, const char *pTextStart )
{
	char *pLine = NULL;
	for ( int i = 0; i < hLines.Count(); i++ )
	{
		delete [] pLine;
		pLine = CopyToDelimiter( hLines[ i ] );

		if ( !pLine || !*pLine || *pLine != '#' )
			continue;

		int iOffset = hLines[ i ] - pTextStart;

		//Msg( "%s\n", pLine );
		if ( bRecursive && CRegex::MatchSyntax_Preproc( pLine ) == __REGEX__PREPROC_INC )
		{
			Parse_Include( pLine, hIncludes );
			continue;
		}

		// handle #define
		if ( CRegex::MatchSyntax_Preproc( pLine ) == __REGEX__PREPROC_DEFINE )
		{
			Parse_Define( pLine, iOffset );
			continue;
		}
	}

	delete [] pLine;
}

//void CSmartObjectList::ParseCode_RemoveInvisibleExpressions( CUtlVector< char* > &hExpressions, int scopeOffset, const char *pTextStart, const int iSize )
//{
//	if ( !hExpressions.Count() )
//		return;
//
//	Assert( hExpressions.Count() && pTextStart <= hExpressions[0] );
//	Assert( scopeOffset <= iSize );
//
//	if ( scopeOffset < 0 )
//		return;
//}

/*
deklarationen nach dem öffnen eines scopes werden ignoriert
deklarationen in impliziten scopes auch (egal?)
expressions werden zB in for(;;) gesplittet (egal?)
*/

void CSmartObjectList::ParseCode_Expressions( CUtlVector< char* > &hExpressions, const char *pTextStart )
{
	char accessModRegexTmp[2048];
	char regexTmp_Var[ sizeof(accessModRegexTmp) ];
	char regexTmp_Func[ sizeof(accessModRegexTmp) ];
	char regexTmp_Struct[ sizeof(accessModRegexTmp) ];
	bool bRegexTmpNeedsRebuild = true;


	int iScopeLevel = 0;
	char *pExp = NULL;

	CUtlVector< int >hScopeCounter;
	hScopeCounter.AddToTail(0);


	for ( int i = 0; i < hExpressions.Count(); i++ )
	{
		delete [] pExp;
		pExp = CopyToDelimiter( hExpressions[ i ], ';' );
		ConvertCharacter( pExp, '\n', ' ' );

		if ( !pExp )
			continue;

		//Warning( "EXP: %s\n", pExp );
		bool bEvaluatedExpression = false;
		char *pScopeReader = pExp;

		while ( pScopeReader && *pScopeReader )
		{
			if ( *pScopeReader == '{' )
			{
				iScopeLevel++;
				bEvaluatedExpression = false;

				if ( iScopeLevel >= hScopeCounter.Count() )
				{
					hScopeCounter.AddToTail( -1 );
				}

				Assert( iScopeLevel < hScopeCounter.Count() );
				hScopeCounter[ iScopeLevel ]++;
			}
			else if ( *pScopeReader == '}' )
			{
				iScopeLevel--;
				bEvaluatedExpression = false;
			}
			else if ( !IS_SPACE( *pScopeReader ) )
			{
				//if ( iScopeLevel != 0 )
				//	bEvaluatedExpression = false;
				//else if ( bEvaluatedExpression == false )
				if ( !IS_BRACKET( *pScopeReader ) && !IS_SPACE( *pScopeReader ) && !bEvaluatedExpression )
				{
					bEvaluatedExpression = true;
#if __DBG_PARSE
					Warning( "%c  ", *pScopeReader );
#endif
					//Msg( "%s\n", pScopeReader );

					if ( bRegexTmpNeedsRebuild )
					{
						CUtlVector< CSmartObject* >hStructs;

						for ( int s = 0; s < hAutoCompUnits.Count(); s++ )
							if ( hAutoCompUnits[s]->m_iType == ACOMP_STRUCT_DEF && hAutoCompUnits[s]->m_pszIdentifier != NULL )
								hStructs.AddToTail( hAutoCompUnits[s] );

						Q_snprintf( accessModRegexTmp, sizeof( accessModRegexTmp ), "(?:" );
						for ( int iKeyword = 0; iKeyword <= __REGEX_KEYWORDS_LASTACCESS_MOD; iKeyword++ )
						{
							Q_strcat( accessModRegexTmp, CRegex::GetSyntaxList_KeyWords()[iKeyword], sizeof( accessModRegexTmp ) );

							if ( iKeyword < __REGEX_KEYWORDS_LASTACCESS_MOD )
								Q_strcat( accessModRegexTmp, "|", sizeof( accessModRegexTmp ) );
							else
								Q_strcat( accessModRegexTmp, "| |\\t)*", sizeof( accessModRegexTmp ) );
						}

						int structNum = 0;
						const int iDefDType = CRegex::GetSyntaxNum_Datatypes_Regex();
						const int iDType = iDefDType + hStructs.Count();

						Q_strcat( accessModRegexTmp, "(?:", sizeof( accessModRegexTmp ) );
						for ( int iDatatype = 0; iDatatype < iDType; iDatatype++ )
						{
							if ( iDatatype < iDefDType )
								Q_strcat( accessModRegexTmp, CRegex::GetSyntaxList_Datatypes_Regex()[iDatatype], sizeof( accessModRegexTmp ) );
							else
							{
								char *pszSafe = CRegex::AllocSafeString( hStructs[structNum]->m_pszIdentifier );
								if ( pszSafe )
									Q_strcat( accessModRegexTmp, pszSafe, sizeof( accessModRegexTmp ) );
								delete [] pszSafe;
								structNum++;
							}

							if ( iDatatype < iDType - 1 )
								Q_strcat( accessModRegexTmp, "|", sizeof( accessModRegexTmp ) );
							else
								Q_strcat( accessModRegexTmp, ")", sizeof( accessModRegexTmp ) );
						}

						//Q_snprintf( regexTmp_Var, sizeof( regexTmp_Var ), "%s[ \\t]+[\\w\\d]+(?:[ \\t]*[^\\( \\t].*)?", accessModRegexTmp );
						Q_snprintf( regexTmp_Var, sizeof( regexTmp_Var ), "%s[ \\t]+[\\w\\d]+.*?", accessModRegexTmp );
						Q_snprintf( regexTmp_Func, sizeof( regexTmp_Func ), "%s[ \\t]+[\\w\\d]+[ \\t]*\\((?:[[:print:]\\t]+)?\\)[[:print:]\\t]*", accessModRegexTmp );
						Q_snprintf( regexTmp_Struct, sizeof( regexTmp_Struct ), "struct[ \\t]+[\\w\\d]+[ \\t]+\\{.*" );

						bRegexTmpNeedsRebuild = false;
						hStructs.Purge();
					}

						// const bool g_bHighQualityShadows : register( b0 )
						// (const|static| |\\t){0,}double([1-4]??|[1-4]x[1-4])   [ \\t]{1,}[a-zA-Z0-9_]{1,}[ \\t]{0,}[\\=\\:]?\0

					Assert( !iScopeLevel || iScopeLevel < hScopeCounter.Count() );

					const int iOffset = hExpressions[ i ] - pTextStart + pScopeReader - pExp;
					const int iScopeDepth = iScopeLevel;
					const int iScopeNum = iScopeLevel ? hScopeCounter[ iScopeLevel ] : 0;

					if ( CRegex::RegexMatch( pScopeReader, regexTmp_Struct ) )
					{
						ParseDbg( "got STRUCT: %s", pScopeReader );
						Parse_Struct( hExpressions[ i ] + ( pScopeReader - pExp ), iOffset, iScopeDepth, iScopeNum );
						bRegexTmpNeedsRebuild = true;
					}
					else if ( CRegex::RegexMatch( pScopeReader, regexTmp_Func ) )
					{
						ParseDbg( "got FUNC: %s", pScopeReader );
						const int inlineLevel = iScopeDepth + 1;
						if ( inlineLevel >= hScopeCounter.Count() )
							hScopeCounter.AddToTail( -1 );
						Assert( inlineLevel < hScopeCounter.Count() );
						int inlineNum = hScopeCounter[ inlineLevel ] + 1;

						Parse_Function( pScopeReader, iOffset, iScopeDepth, iScopeNum, inlineLevel, inlineNum );
					}
					else if ( CRegex::RegexMatch( pScopeReader, regexTmp_Var ) )
					{
						ParseDbg( "got VAR: %s", pScopeReader );
						Parse_Variable( pScopeReader, iOffset, iScopeDepth, iScopeNum );
					}
					else
					{
						ParseDbg( "UNHANDLED: %s", pScopeReader );
					}

					//pScopeReader = NULL;
					//break;
				}
			}

			pScopeReader++;
		}
	}

	hScopeCounter.Purge();
	delete [] pExp;
}

void CSmartObjectList::ParseCode_KillPreProc( CUtlVector< char* > &hLines )
{
	for ( int i = 0; i < hLines.Count(); i++ )
	{
		char *pReadLine = hLines[i];

		while ( *pReadLine && IS_SPACE(*pReadLine) )
			pReadLine++;

		if ( *pReadLine == '\n' )
		{
			// these should have been removed already
			Assert( 0 );
			continue;
		}

		if ( *pReadLine == '#' )
		{
			// assume preproc statement, kill it
			while ( *pReadLine && *pReadLine != '\n' )
			{
				// expression iterator will jump over LF and may hit this stuff again
				*pReadLine = ' ';
				pReadLine++;
			}

			hLines.Remove( i );
			i--;
			continue;
		}
	}
}

void CSmartObjectList::ParseCode_RemoveComments( CUtlBuffer &hStream )
{
	char *pText = (char*)hStream.Base();
	char *pRead_ClearCR = pText;

	const int Size = hStream.Size();
	int iSizeItr = Size;

	bool bInSingleLineCmt = false;
	bool bInMultiLineCmt = false;

	while ( pRead_ClearCR && *pRead_ClearCR && iSizeItr > 0 )
	{
		//if ( *pRead_ClearCR == '\r' )
		//	*pRead_ClearCR = ' ';

		if ( bInSingleLineCmt )
		{
			if ( *pRead_ClearCR == '\n' )
				bInSingleLineCmt = false;
			else
				*pRead_ClearCR = ' ';
		}
		else if ( bInMultiLineCmt )
		{
			if ( iSizeItr > 1 )
			{
				if ( *(pRead_ClearCR) == '*' && *(pRead_ClearCR+1) == '/' )
				{
					*(pRead_ClearCR+1) = ' ';
					bInMultiLineCmt = false;
				}

				*pRead_ClearCR = ' ';
			}
		}
		else if ( iSizeItr > 1 )
		{
			if ( *pRead_ClearCR == '/' && *(pRead_ClearCR+1) == '/' )
			{
				bInSingleLineCmt = true;
				*pRead_ClearCR = ' ';
			}
			else if ( *pRead_ClearCR == '/' && *(pRead_ClearCR+1) == '*' )
			{
				bInMultiLineCmt = true;
				*pRead_ClearCR = ' ';
				*(pRead_ClearCR+1) = ' ';
			}
		}

		pRead_ClearCR++;
		iSizeItr--;
	}
}

static const char *pszDatatypes[] = {
	"double",
	"float",
	"half",
	"HALF",
	"int",
	"uint",
	"bool",

	"sampler",
	"Buffer",
	"matrix",
	"vector",
};
static const int iDatatypesSize = ARRAYSIZE( pszDatatypes );

static const char *vecLookUpIdents[] =
{
	"x", "y", "z", "w",
};

CSmartObject *AllocIntrinsicChild( int datatype, int index )
{
	Assert( datatype >= 0 && datatype < iDatatypesSize );
	Assert( index >= 0 && index < 4 );

	char tmp[MAX_PATH];
	Q_snprintf( tmp, sizeof(tmp), "%s %s", pszDatatypes[datatype], vecLookUpIdents[index] );
	CSmartObject *pO = new CSmartObject( ACOMP_VAR, tmp, vecLookUpIdents[index], pszDatatypes[datatype] );
	pO->m_bIntrinsic = true;
	return pO;
}


void CSmartObjectList::BuildIntrinsicDatatypes()
{
	const int iFirstLocked = 7;

	char tmp[MAX_PATH];
	char tmp_dtype[MAX_PATH];
	char tmp_child[MAX_PATH];

	for ( int i = 0; i < iDatatypesSize; i++ )
	{
		if ( i < iFirstLocked )
		{
			Q_snprintf( tmp, sizeof(tmp), "%s( %s x )", pszDatatypes[i], pszDatatypes[i] );

			CSmartObject *pO = new CSmartObject( ACOMP_FUNC, tmp, pszDatatypes[i], pszDatatypes[i] );
			pO->m_bIntrinsic = true;
			pO->m_hChildren.AddToTail( AllocIntrinsicChild( i, 0 ) );
			AddUnit( pO );

			for ( int d1 = 1; d1 <= 4; d1++ )
			{
				Q_snprintf( tmp, sizeof(tmp), "%s%d(", pszDatatypes[i], d1 );

				for ( int child = 0; child < d1; child++ )
				{
					Q_snprintf( tmp_child, sizeof(tmp_child), " %s %s", pszDatatypes[i], vecLookUpIdents[child] );
					Q_strcat( tmp, tmp_child, sizeof(tmp) );
					Q_strcat( tmp, (child<d1-1)? ",":" ", sizeof(tmp) );
				}

				Q_strcat( tmp, ")", sizeof(tmp) );

				Q_snprintf( tmp_dtype, sizeof(tmp_dtype), "%s%d", pszDatatypes[i], d1 );
				
				CSmartObject *pO = new CSmartObject( ACOMP_FUNC, tmp, tmp_dtype, tmp_dtype );
				pO->m_bIntrinsic = true;
				for ( int child = 0; child < d1; child++ )
					pO->m_hChildren.AddToTail( AllocIntrinsicChild( i, child ) );
				AddUnit( pO );
			}
		}
		else
		{
			CSmartObject *pO = new CSmartObject( ACOMP_VAR, pszDatatypes[i], pszDatatypes[i] );
			pO->m_bIntrinsic = true;
			AddUnit( pO );
		}
	}
}

#ifndef SHADER_EDITOR_DLL_2006
CON_COMMAND( test_parse, "" )
{
	CSmartObjectList l;

	int idx = ( args.ArgC() > 1 ) ? atoi( args[ 1 ] ) : 0;

	CUtlVector< char* > hIncludes;

	const char *pfile = "common_ps_fxc.h";
	switch ( idx )
	{
	case 1:
			pfile = "common_vertexlitgeneric_dx9.h";
		break;
	case 2:
			pfile = "common_lightmappedgeneric_fxc.h";
		break;
	case 3:
			pfile = "common_flashlight_fxc.h";
		break;
	case 4:
			pfile = "common_parallax.h";
		break;
	case 5:
			pfile = "common_vs_fxc.h";
		break;
	case 6:
			pfile = ".h";
		break;
	}
	l.ParseCode( pfile, true, &hIncludes );

	for ( int i = 0; i < hIncludes.Count(); i++ )
		delete [] hIncludes[ i ];

	hIncludes.Purge();
}
#endif