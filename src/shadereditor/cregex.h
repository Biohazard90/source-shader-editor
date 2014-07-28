#ifndef C_REGEX_H
#define C_REGEX_H


#define IS_CONTROL_CHAR( x ) ( x == '\n' || x == '\r' )
#define IS_CHAR( x ) ( x >= L'a' && x <= L'z' || x >= L'A' && x <= L'Z' || x == L'#' || x == L'_' )
#define IS_NUMERICAL( x ) ( x >= L'0' && x <= L'9' ) //|| x == '-' ) ) //|| x == '.' )
#define IS_SPACE( x ) ( x == L' ' || x == L'\t' || x == L'\r' )

#define IS_BRACKET_OPENING( x ) ( x == L'(' || x == L'[' || x == L'{' )
#define IS_BRACKET_CLOSING( x ) ( x == L')' || x == L']' || x == L'}' )
#define IS_BRACKET( x ) ( IS_BRACKET_OPENING( x ) || IS_BRACKET_CLOSING( x ) )
#define IS_BRACKET_SAME_TYPE( x, y ) ( x == y ||\
										( ( x == L'(' || x == L')' ) && ( y == L'(' || y == L')' ) ) ||\
										( ( x == L'[' || x == L']' ) && ( y == L'[' || y == L']' ) ) ||\
										( ( x == L'{' || x == L'}' ) && ( y == L'{' || y == L'}' ) ) )

#define __REGEX_INVALID -1

#define __REGEX_KEYWORDS_LASTACCESS_MOD 10

enum
{
	__REGEX__PREPROC_DEFINE = 0,
	__REGEX__PREPROC_IF_DEFINED,
	__REGEX__PREPROC_IF,
	__REGEX__PREPROC_IFNDEF,
	__REGEX__PREPROC_IFDEF,
	__REGEX__PREPROC_ELIF,
	__REGEX__PREPROC_ELSE,
	__REGEX__PREPROC_ENDIF,
	__REGEX__PREPROC_UNDEF,
	__REGEX__PREPROC_INC,
	__REGEX__PREPROC_,
};

class CRegex
{
public:

	static bool RegexMatch( const wchar_t *lStr, const char *exp );
	static bool RegexMatch( const char *lStr, const char *exp );
	static const Color *GetColorForWordBySyntax( const wchar_t *pWord, bool bStringLiterals = true, bool bBrackets = true, bool bHLSL = true );
	static char *AllocSafeString( char *str );


	static int MatchSyntax_KeyWord( const char *pWord );
	static int MatchSyntax_KeyWord( const wchar_t *pWord );

	static int MatchSyntax_Semantic( const char *pWord );
	static int MatchSyntax_Semantic( const wchar_t *pWord );

	static int MatchSyntax_Preproc( const char *pWord );
	static int MatchSyntax_Preproc( const wchar_t *pWord );

	static int MatchSyntax_Datatypes( const char *pWord );
	static int MatchSyntax_Datatypes( const wchar_t *pWord );

	static int MatchSyntax_Numbers( const char *pWord );
	static int MatchSyntax_Numbers( const wchar_t *pWord );


	static const char **GetSyntaxList_KeyWords();
	static const int &GetSyntaxNum_KeyWords();

	static const char **GetSyntaxList_Semantics();
	static const int &GetSyntaxNum_Semantics();

	static const char **GetSyntaxList_Preproc_Regex();
	static const int &GetSyntaxNum_Preproc_Regex();

	static const char **GetSyntaxList_Datatypes_Regex();
	static const int &GetSyntaxNum_Datatypes_Regex();

	static const char **GetSyntaxList_Semantics_Regex();
	static const int &GetSyntaxNum_Semantics_Regex();

	static const char *GetSyntaxRegex_Numbers();
};
#endif