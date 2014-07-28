
#include "cbase.h"
#include "editorCommon.h"


char *CKVPacker::ConvertKVSafeString( const char *pStr, bool bMakeEscaped )
{
	//const bool MakeCopy = true;

	if ( !pStr || !*pStr )
		return NULL;

	bool b_LF, b_CR, b_Backslash, b_DQuote;

	int iNewSize = 1;
	const char *pRead = pStr;
	bool bWasEscaping = false;
	while ( *pRead )
	{
		b_LF = *pRead == '\n';
		b_CR = *pRead == '\r';
		b_Backslash = *pRead == '\\';
		b_DQuote = *pRead == '"';

		if ( bMakeEscaped &&
			(b_LF || b_CR || b_Backslash || b_DQuote) )
			iNewSize++;
		else if ( !bMakeEscaped &&
			b_Backslash && !bWasEscaping ) //&&
		{
			//(*(pRead+1) == 'n') )
			iNewSize--;
			bWasEscaping = true;
		}
		else
			bWasEscaping = false;
		pRead++;
		iNewSize++;
	}

	char *pOut = new char[ iNewSize ];
	pRead = pStr;
	char *pWrite = pOut;

	while ( *pRead )
	{
		b_LF = *pRead == '\n';
		b_CR = *pRead == '\r';
		b_Backslash = *pRead == '\\';
		b_DQuote = *pRead == '"';

		if ( bMakeEscaped && 
			(b_LF || b_CR || b_Backslash || b_DQuote) )
		{
			*pWrite = '\\';
			pWrite++;

			if ( b_LF )
				*pWrite = 'n';
			else if ( b_CR )
				*pWrite = 'r';
			else if ( b_DQuote )
				*pWrite = '\'';
			else
				*pWrite = '\\';
		}
		else if ( !bMakeEscaped &&
			b_Backslash )
		{
			pRead++;

			if ( *pRead == 'n' )
				*pWrite = '\n';
			else if ( *pRead == 'r' )
				*pWrite = '\r';
			else if ( *pRead == '\'' )
				*pWrite = '"';
			else
				*pWrite = *pRead;
		}
		else
			*pWrite = *pRead;

		pRead++;
		pWrite++;
	}

	Assert( iNewSize - 1 == pWrite - pOut );

	*pWrite = '\0';

	//if ( !MakeCopy )
	//	delete [] pStr;

	return pOut;
}

#define KVPACKER_MAX_CHARS_PER_STRING 1023
#define KVPACKER_AMT_STR "_num_strings"
#define KVPACKER_PARTIAL_STR "_partial_"

bool CKVPacker::KVPack( const char *pszStr, const char *pszKey, KeyValues *pKVTarget )
{
	if ( !pszStr || !pszKey || !*pszKey )
		return false;

	const int iStrLen = Q_strlen( pszStr );
	const int iNumStrings = iStrLen / KVPACKER_MAX_CHARS_PER_STRING + 1;

	char tmpString[ KVPACKER_MAX_CHARS_PER_STRING + 1 ];
	const char *pRead = pszStr;

	char tmpKey[MAXTARGC];
	Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s", pszKey, KVPACKER_AMT_STR );
	pKVTarget->SetInt( tmpKey, iNumStrings );

	for ( int i = 0; i < iNumStrings; i++ )
	{
		Assert( pRead - pszStr <= iStrLen );

		Q_snprintf( tmpString, sizeof(tmpString), "%s", pRead );
		Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s%02i", pszKey, KVPACKER_PARTIAL_STR, i );
		pKVTarget->SetString( tmpKey, tmpString );

		pRead += KVPACKER_MAX_CHARS_PER_STRING;
	}

	return true;
}

char *CKVPacker::KVUnpack( KeyValues *pKVSource, const char *pszKey )
{
	if ( !pKVSource || !pszKey || !*pszKey )
		return NULL;

	char tmpKey[MAXTARGC];
	Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s", pszKey, KVPACKER_AMT_STR );
	const int numEntries = pKVSource->GetInt( tmpKey );

	if ( numEntries <= 0 )
		return NULL;

	int iStrlen = 1;
	CUtlVector< const char* > hRowList;

	for ( int i = 0; i < numEntries; i++ )
	{
		Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s%02i", pszKey, KVPACKER_PARTIAL_STR, i );
		const char *pszStr = pKVSource->GetString( tmpKey );

		iStrlen += Q_strlen( pszStr );
		hRowList.AddToTail( pszStr );
	}

	char *pszOut = new char[ iStrlen ];
	*pszOut = '\0';

	for ( int i = 0; i < hRowList.Count(); i++ )
	{
		Q_strcat( pszOut, hRowList[i], iStrlen );
	}

	hRowList.Purge();

	return pszOut;
}

bool CKVPacker::KVCopyPacked( KeyValues *pKVSource, KeyValues *pKVTarget, const char *pszKey )
{
	if ( !pKVSource || !pKVTarget )
		return false;

	char tmpKey[MAXTARGC];
	Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s", pszKey, KVPACKER_AMT_STR );
	const int iNumStrings = pKVSource->GetInt( tmpKey );

	pKVTarget->SetInt( tmpKey, iNumStrings );

	int i = 0;
	for ( ; i < iNumStrings; i++ )
	{
		Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s%02i", pszKey, KVPACKER_PARTIAL_STR, i );
		pKVTarget->SetString( tmpKey, pKVSource->GetString( tmpKey ) );
	}

	Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s%02i", pszKey, KVPACKER_PARTIAL_STR, i );

	for ( KeyValues *pTerminateMe = pKVTarget->FindKey( tmpKey ); pTerminateMe != NULL;
		i++, Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s%02i", pszKey, KVPACKER_PARTIAL_STR, i ),
		pTerminateMe = pKVTarget->FindKey( tmpKey ) )
	{
		pKVTarget->RemoveSubKey( pTerminateMe );
		pTerminateMe->deleteThis();
	}

	return true;
}

void CKVPacker::KVClearPacked( const char *pszKey, KeyValues *pKVTarget )
{
	if ( !pszKey || !*pszKey || !pKVTarget )
		return;

	char tmpKey[MAXTARGC];
	Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s", pszKey, KVPACKER_AMT_STR );
	pKVTarget->SetInt( tmpKey, 0 );

	int i = 0;

	Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s%02i", pszKey, KVPACKER_PARTIAL_STR, i );
	for ( KeyValues *pTerminateMe = pKVTarget->FindKey( tmpKey ); pTerminateMe != NULL;
		i++, Q_snprintf( tmpKey, sizeof(tmpKey), "%s%s%02i", pszKey, KVPACKER_PARTIAL_STR, i ),
		pTerminateMe = pKVTarget->FindKey( tmpKey ) )
	{
		pKVTarget->RemoveSubKey( pTerminateMe );
		pTerminateMe->deleteThis();
	}
}