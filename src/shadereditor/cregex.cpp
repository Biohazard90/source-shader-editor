
#include "cbase.h"
#include "editorcommon.h"

#ifdef new
#undef new
#endif

#include <regex>
#include "cRegex.h"

static const char *g_szSyntaxList_Keywords[] =
{
	"static",
	"extern",
	"nointerpolation",
	"precise",
	"shared",
	"groupshared",
	"uniform",
	"volatile",

	"const",
	"row_major",
	"column_major",

	"register",
	"return",
	"struct",

	"for",
	"do",
	"while",
	"if",
	"else",

	"true",
	"false",

	"in",
	"out",
	"inout",
};
static const int g_iNum_SyntaxList_Keywords = ARRAYSIZE( g_szSyntaxList_Keywords );

static const char *g_szSyntaxList_Preproc_Regex[] =
{
	"#[ \\t]*define(?:[ \\t]+.*)?",
	"#[ \\t]*if[ \\t]+defined(?:[ \\t]+.*)?",
	"#[ \\t]*if(?:[ \\t]{1,}.*)?",
	"#[ \\t]*ifndef(?:[ \\t]+.*)?",
	"#[ \\t]*ifdef(?:[ \\t]+.*)?",
	"#[ \\t]*elif(?:[ \\t]+.*)?",
	"#[ \\t]*else(?:[ \\t]+.*)?",
	"#[ \\t]*endif(?:[ \\t]+.*)?",
	"#[ \\t]*undef(?:[ \\t]+.*)?",
	"#[ \\t]*include(?:[ \\t]+.*)?",
};
static const int g_iNum_SyntaxList_Preproc_Regex = ARRAYSIZE( g_szSyntaxList_Preproc_Regex );

static const char *g_szSyntaxList_Datatypes_Regex[] =
{
	"double(?:[1-4]?|[1-4]x[1-4])",
	"float(?:[1-4]?|[1-4]x[1-4])",
	"half(?:[1-4]?|[1-4]x[1-4])",
	"int(?:[1-4]?|[1-4]x[1-4])",
	"uint(?:[1-4]?|[1-4]x[1-4])",
	"bool(?:[1-4]?|[1-4]x[1-4])",
	"HALF(?:[1-4]?|[1-4]x[1-4])",
	"void",
	"sampler(?:[1-3]D|CUBE)?",
	"Buffer",

	"matrix",
	"vector",

	"auto",
	"multi",

	//"matrix[ \\t]{0,}<[ \\t]{0,}(double|float|half|HALF|int|uint|bool)[ \\t]{0,},[ \\t]{0,}[1-4][ \\t]{0,},[ \\t]{0,}[1-4][ \\t]{0,}>",
	//"vector[ \\t]{0,}<[ \\t]{0,}(double|float|half|HALF|int|uint|bool)[ \\t]{0,},[ \\t]{0,}[1-4][ \\t]{0,}>",
};
static const int g_iNum_SyntaxList_Datatypes_Regex = ARRAYSIZE( g_szSyntaxList_Datatypes_Regex );

static const char *g_szSyntaxList_Semantics[] =
{
	"POSITIONT",
	"FOG",
	"PSIZE",
	"VFACE",
	"VPOS",
};
static const int g_iNum_SyntaxList_Semantics = ARRAYSIZE( g_szSyntaxList_Semantics );

static const char *g_szSyntaxList_Semantics_Regex[] =
{
	"TEXCOORD\\d*",
	"BINORMAL\\d*",
	"BLENDINDICES\\d*",
	"BLENDWEIGHT\\d*",
	"COLOR\\d*",
	"NORMAL\\d*",
	"POSITION\\d*",
	"TANGENT\\d*",
	"TESSFACTOR\\d*",
	"DEPTH\\d*",
};
static const int g_iNum_SyntaxList_Semantics_Regex = ARRAYSIZE( g_szSyntaxList_Semantics_Regex );

const char *g_szSyntax_Number_Regex = "^\\w?-?\\d*.?\\d+[eE]?-?\\d*[hHfFlLuU]?";

static char *__AllocCChar( const wchar_t *wC )
{
	const int len = Q_wcslen( wC ) + 1;
	char *c = new char[ len ];
	Q_UnicodeToUTF8( wC, c, len );
	c[len-1] = '\0';
	return c;
}

int CRegex::MatchSyntax_KeyWord( const char *pWord )
{
	for ( int i = 0; i < GetSyntaxNum_KeyWords(); i++ )
		if ( !Q_strcmp( pWord, GetSyntaxList_KeyWords()[i] ) )
			return i;
	return -1;
}
int CRegex::MatchSyntax_KeyWord( const wchar_t *pWord )
{
	char *c = __AllocCChar( pWord );
	int r = MatchSyntax_KeyWord( c );
	delete [] c;
	return r;
}

int CRegex::MatchSyntax_Semantic( const char *pWord )
{
	int i = 0;
	for ( ; i < GetSyntaxNum_Semantics(); i++ )
		if ( !Q_strcmp( pWord, GetSyntaxList_Semantics()[i] ) )
			return i;
	for ( int i1 = 0; i1 < GetSyntaxNum_Semantics_Regex(); i1++ )
		if ( RegexMatch( pWord, GetSyntaxList_Semantics_Regex()[i1] ) )
			return i + i1;
	return -1;
}
int CRegex::MatchSyntax_Semantic( const wchar_t *pWord )
{
	char *c = __AllocCChar( pWord );
	int r = MatchSyntax_Semantic( c );
	delete [] c;
	return r;
}

int CRegex::MatchSyntax_Preproc( const char *pWord )
{
	for ( int i = 0; i < GetSyntaxNum_Preproc_Regex(); i++ )
		if ( RegexMatch( pWord, GetSyntaxList_Preproc_Regex()[i] ) )
			return i;
	return -1;
}
int CRegex::MatchSyntax_Preproc( const wchar_t *pWord )
{
	char *c = __AllocCChar( pWord );
	int r = MatchSyntax_Preproc( c );
	delete [] c;
	return r;
}

int CRegex::MatchSyntax_Datatypes( const char *pWord )
{
	for ( int i = 0; i < GetSyntaxNum_Datatypes_Regex(); i++ )
		if ( RegexMatch( pWord, GetSyntaxList_Datatypes_Regex()[i] ) )
			return i;
	return -1;
}
int CRegex::MatchSyntax_Datatypes( const wchar_t *pWord )
{
	char *c = __AllocCChar( pWord );
	int r = MatchSyntax_Datatypes( c );
	delete [] c;
	return r;
}


int CRegex::MatchSyntax_Numbers( const char *pWord )
{
	return RegexMatch( pWord, GetSyntaxRegex_Numbers() ) ? 1 : -1;
}
int CRegex::MatchSyntax_Numbers( const wchar_t *pWord )
{
	char *c = __AllocCChar( pWord );
	int r = MatchSyntax_Numbers( c );
	delete [] c;
	return r;
}

const char **CRegex::GetSyntaxList_KeyWords(){ return g_szSyntaxList_Keywords; };
const int &CRegex::GetSyntaxNum_KeyWords(){ return g_iNum_SyntaxList_Keywords; };

const char **CRegex::GetSyntaxList_Semantics(){ return g_szSyntaxList_Semantics; };
const int &CRegex::GetSyntaxNum_Semantics(){ return g_iNum_SyntaxList_Semantics; };

const char **CRegex::GetSyntaxList_Preproc_Regex(){ return g_szSyntaxList_Preproc_Regex; };
const int &CRegex::GetSyntaxNum_Preproc_Regex(){ return g_iNum_SyntaxList_Preproc_Regex; };

const char **CRegex::GetSyntaxList_Datatypes_Regex(){ return g_szSyntaxList_Datatypes_Regex; };
const int &CRegex::GetSyntaxNum_Datatypes_Regex(){ return g_iNum_SyntaxList_Datatypes_Regex; };

const char **CRegex::GetSyntaxList_Semantics_Regex(){ return g_szSyntaxList_Semantics_Regex; };
const int &CRegex::GetSyntaxNum_Semantics_Regex(){ return g_iNum_SyntaxList_Semantics_Regex; };

const char *CRegex::GetSyntaxRegex_Numbers(){ return g_szSyntax_Number_Regex; };

bool CRegex::RegexMatch( const char *cStr, const char *exp )
{
	//std::string stdStr;
	//stdStr.append( cStr );

	std::tr1::regex reg( exp );
	//return std::tr1::regex_match( stdStr.begin(), stdStr.end(), reg, std::tr1::regex_constants::match_default );
	return std::tr1::regex_match( cStr, reg, std::tr1::regex_constants::match_default );
}
bool CRegex::RegexMatch( const wchar_t *lStr, const char *exp )
{
	char *cStr = __AllocCChar( lStr );
	bool r = RegexMatch( cStr, exp );
	delete [] cStr;
	return r;
}
bool CharNeedsEscape( char c )
{
	if ( c == '*' ||
		c == '.' ||
		c == '\\' ||
		c == '(' ||
		c == ')' ||
		c == '[' ||
		c == ']' ||
		c == '{' ||
		c == '}' ||
		c == '$' ||
		c == '^' ||
		c == '+' ||
		c == '?' ||
		c == '-' ||
		c == ',' ||
		c == ':' ||
		c == '=' ||
		c == '!' )
		return true;

	return false;
}
char *CRegex::AllocSafeString( char *str )
{
	if ( !str || !*str )
		return NULL;

	int len = 1;

	char *walk = str;
	while ( *walk )
	{
		if ( CharNeedsEscape( *walk ) )
			len++;

		len++;
		walk++;
	}

	char *pOut = new char[ len ];

	walk = str;
	char *pWrite = pOut;

	while ( *walk )
	{
		if ( CharNeedsEscape( *walk ) )
		{
			*pWrite = '\\';
			pWrite++;
		}

		*pWrite = *walk;

		pWrite++;
		walk++;
	}

	*pWrite = '\0';

	return pOut;
}


static const Color g_ColSyntax_Keyword( 90, 210, 0, 255 );
static const Color g_ColSyntax_Semantic( 40, 170, 80, 255 );
static const Color g_ColSyntax_Preproc( 160, 80, 40, 255 );
static const Color g_ColSyntax_Number( 255, 128, 32, 255 );
static const Color g_ColSyntax_String( 200, 160, 16, 255 );
static const Color g_ColSyntax_Bracket( 210, 160, 70, 255 );

const Color *CRegex::GetColorForWordBySyntax( const wchar_t *pWord, bool bStringLiterals, bool bBrackets, bool bHLSL )
{
	if ( !pWord || !*pWord )
		return NULL;

	// number literals
	if ( IS_NUMERICAL( *pWord ) || *pWord == L'.' || *pWord == L'-' )
	{
		if ( MatchSyntax_Numbers( pWord ) != __REGEX_INVALID )
			return &g_ColSyntax_Number;
	}

	if ( bStringLiterals )
	{
		// strings
		if ( *pWord == L'\"' || *pWord == L'\'' )
			return &g_ColSyntax_String;
	}

	if ( bBrackets )
	{
		// brackets
		if ( ( IS_BRACKET( *pWord ) ) && //|| *pWord == L'<' || *pWord == L'>' ) &&
			Q_wcslen( pWord ) == 1 )
			return &g_ColSyntax_Bracket;
	}

	if ( bHLSL )
	{
		// datatypes/keywords
		if ( MatchSyntax_KeyWord( pWord ) != __REGEX_INVALID )
			return &g_ColSyntax_Keyword;

		// semantics
		if ( MatchSyntax_Semantic( pWord ) != __REGEX_INVALID )
			return &g_ColSyntax_Semantic;

		// datatypes/keywords, regex
		int match_dtype = MatchSyntax_Datatypes( pWord );
		if ( match_dtype != __REGEX_INVALID && match_dtype < 12 )
			return &g_ColSyntax_Keyword;

		// pre processor
		if ( MatchSyntax_Preproc( pWord ) != __REGEX_INVALID )
			return &g_ColSyntax_Preproc;
	}

	return NULL;
}