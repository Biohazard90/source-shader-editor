
#include "vgui_controls/Utilities.h"
#include "vgui_controls/tokenchannels.h"

#include "vgui_editor_platform.h"

#define MAX_OPERATION_C 1024


#define ISCHAR_OPERATOR_STEP1( x ) ( x == '*' ||\
								x == '/' )
#define ISCHAR_OPERATOR_STEP2( x ) ( x == '+' ||\
								x == '-' )
#define ISSTRING_OPERATOR( x ) ( ISCHAR_OPERATOR_STEP1(x[0]) ||\
								ISCHAR_OPERATOR_STEP2(x[0]) )

#define ISSTRING_LITERAL( start, x ) ( ( Q_strlen(x) > 1 && x[0] == 'P' && x[1] == 'I' ) ||\
								( x[0] == 'I' && x > start && x[-1] == 'P' ) ||\
								x[0] == 'E' ||\
								(x[0] == '-' && (x[1] == 'P' || x[1] == 'E')) )

#define ISCHAR_VALUE( x ) ( x >= '0' && x <= '9' )
#define ISSTRING_VALUE( start, x ) ( ISCHAR_VALUE( x[0] ) ||\
							x[0] == '.' ||\
							(x[0] == '-' && ( (ISCHAR_VALUE(x[1])) || (x > start && ISSTRING_OPERATOR( (x-1) ) ) ) ) )

#define ISCHAR_BOPEN( x ) ( x == '(' )
#define ISCHAR_BCLOSE( x ) ( x == ')' )
#define ISCHAR_BRACKET( x ) ( ISCHAR_BOPEN(x) ||\
							ISCHAR_BCLOSE(x) )



void SnipCharFromString( char *c )
{
	if ( !c || !*c )
		return;

	char tmp[MAX_OPERATION_C];
	Q_memset( tmp, 0, MAX_OPERATION_C );
	Q_strcpy( tmp, c + 1 );
	Q_strcpy( c, tmp );
}
float GetValueFromChar( const char *stringstart, const char *c )
{
	const bool bLiteralUnsigned = ISSTRING_LITERAL( stringstart, c );
	const bool bLiteralSigned = ISSTRING_LITERAL( stringstart, (c+1) );
	if ( bLiteralUnsigned ||
		( *c == '-' && bLiteralSigned ) )
	{
		if ( *c == '-' )
			c++;
		float sign = 1;
		const char *testsign = c - 1;
		const char *testsign2 = c - 2;
		if ( c > stringstart && *testsign == '-' && ( (c-2)<stringstart || ISSTRING_OPERATOR(testsign2) ) )
			sign = -1;
		if ( *c == 'P' )
			return M_PI * sign;
		if ( *c == 'E' )
			return 2.7182818284f * sign;
	}
	else if ( ISSTRING_VALUE( stringstart, c ) )
		return atof( c );
	return 0;
}
void SolveOperator( char *psz_StringStart, char *psz_Op )
{
	char op = *psz_Op;

	char *firstValue = psz_Op-1;
	char *secondValue = psz_Op+1;
	char *operatorEnd = secondValue;
	while ( firstValue >= psz_StringStart )
	{
		const char *tmpNext = (firstValue > psz_StringStart) ? firstValue - 1 : NULL;
		if ( ISSTRING_OPERATOR( firstValue ) && ( !tmpNext || !ISSTRING_OPERATOR(tmpNext) ) )
			break;
		firstValue--;
	}
	if ( firstValue != psz_StringStart )
		firstValue++;
	while ( *operatorEnd )
	{
		if ( ISSTRING_OPERATOR( operatorEnd ) )
			break;
		operatorEnd++;
	}
	if ( !*operatorEnd || *operatorEnd != '-' )
		operatorEnd--;

	float var1 = GetValueFromChar( psz_StringStart, firstValue );
	float var2 = ( *secondValue == ' ' ) ? 0 : GetValueFromChar( psz_StringStart, secondValue );
	float var_out = 0;

	switch ( op )
	{
	case '+':
		var_out = var1 + var2;
		break;
	case '-':
		var_out = var1 - var2;
		break;
	case '*':
		var_out = var1 * var2;
		break;
	case '/':
		if ( var2 == 0.0f )
			var2 = 1;
		var_out = var1 / var2;
		break;
	}

	char stringTarget[ MAX_OPERATION_C ];
	Q_snprintf( stringTarget, MAX_OPERATION_C, "%f\0", var_out );

	int numToSnip = operatorEnd - firstValue;
	for ( ;numToSnip;numToSnip-- )
		SnipCharFromString( firstValue );
	*firstValue = '\0';

	char tmp_begin[MAX_OPERATION_C];
	char tmp_end[MAX_OPERATION_C];
	Q_strcpy( tmp_begin, psz_StringStart );
	Q_strcpy( tmp_end, firstValue+1 );
	char finaloutput[MAX_OPERATION_C];
	Q_snprintf( finaloutput, MAX_OPERATION_C, "%s%s%s\0", tmp_begin, stringTarget, tmp_end );
	Q_strcpy( psz_StringStart, finaloutput );
}
void StringReplace( char *dest, int destlen, const char *src )
{
	const char *string_append = dest + destlen;
	char finalOutput[ MAX_OPERATION_C ];

	Q_snprintf( finalOutput, MAX_OPERATION_C, "%s%s\0", src, string_append );
	Q_strcpy( dest, finalOutput );
}
void SolveBracketPair( char *bracket_open, char *bracket_close )
{
	//			(1+2)
	//			1
	Assert( bracket_open && *bracket_open );

	int len_real = bracket_close - bracket_open;

	char *ops_Start = bracket_open;
	char *ops_End = bracket_close;

	if ( ISCHAR_BRACKET( *ops_Start ) && *(ops_Start+1) )
		ops_Start++;
	const bool bIsClosing = ISCHAR_BRACKET( *ops_End );

	int len_operation = ops_End - ops_Start;

	char solveString[ MAX_OPERATION_C ];

	Q_memcpy( solveString, ops_Start, len_operation );
	solveString[ len_operation ] = '\0';

	for ( int step = 0; step < 2; step++ )
	{
		bool didValueLastStep = false;

		for( char *SolveIterate = solveString; *SolveIterate; SolveIterate++ )
		{
			const bool bOps_step1 = ISCHAR_OPERATOR_STEP1( *SolveIterate );
			const bool bOps_step2 = ISCHAR_OPERATOR_STEP2( *SolveIterate );
			const bool bOps_common = bOps_step1 || bOps_step2;
			const bool bOps_cur = step ? bOps_step2 : bOps_step1;

			const bool bValue = ISSTRING_LITERAL( solveString, SolveIterate ) || ISSTRING_VALUE( solveString, SolveIterate );
			if ( bValue && !bOps_cur )
				didValueLastStep = true;
			else if ( bOps_cur && didValueLastStep )
			{
				SolveOperator( solveString, SolveIterate );
				SolveIterate = solveString;
				didValueLastStep = false;
			}
			else if ( !bValue && !bOps_common )
			{
				SnipCharFromString( SolveIterate );
			}
		}
	}

	float finalValue = GetValueFromChar( solveString, solveString );
	char valueString[ MAX_OPERATION_C ];
	Q_snprintf( valueString, MAX_OPERATION_C, "%f\0", finalValue );
	StringReplace( bracket_open, len_real + bIsClosing, valueString );
}
void OnBracketOpen( char *bOpen )
{
	if ( !bOpen || !*bOpen )
		return;

	char *readBracketOpen = bOpen + 1;

	while( *readBracketOpen )
	{
		if ( ISCHAR_BCLOSE( *readBracketOpen ) )
			break;
		if ( ISCHAR_BOPEN( *readBracketOpen ) )
		{
			char *startRead = readBracketOpen;
			OnBracketOpen( startRead );
		}
		readBracketOpen++;
	}

	char *readBracketClose = bOpen;
	while ( *readBracketClose )
	{
		if ( ISCHAR_BCLOSE( *readBracketClose ) )
			break;
		readBracketClose++;
	}

	SolveBracketPair( bOpen, readBracketClose );

	bool bFinished = true;
	char *readOperators = bOpen;
	while(*readOperators)
	{
		if ( ISSTRING_OPERATOR( readOperators ) && !ISSTRING_VALUE( bOpen, readOperators ) )
			bFinished = false;
		readOperators++;
	}
	if ( !bFinished )
		OnBracketOpen( bOpen );
}
float TokenChannels_t::PerformSolve( const char *start )
{
	if ( !start || !*start )
		return 0;

	char solveString[MAX_OPERATION_C];
	Q_snprintf( solveString, sizeof( solveString ), "%s\0", start );

	OnBracketOpen( solveString );
	return GetValueFromChar( solveString, solveString );
}

TokenChannels_t::TokenChannels_t()
{
	bHasAnyValues = false;
	bReady = false;
	for ( int i = 0; i < 4; i++ )
	{
		bChannelAsValue[i] = false;
		cChannel[i] = 'x';
		iChannel[i] = 0;
		flValue[i] = 0;
	}
	iChannelDepth = 0;
	iActiveChannels = 0;
	bAllowChars = true;
}
const bool TokenChannels_t::IsActive() const
{
	return bReady;
}
void TokenChannels_t::SetDefaultChannels()
{
	bReady = false;
	bHasAnyValues = false;
	iChannelDepth = 0;
	iActiveChannels = 0;
	for ( int i = 0; i < 4; i++ )
	{
		bChannelAsValue[i] = false;
		cChannel[i] = GetCharFromChannelNum( i );
		iChannel[ i ] = i;
		flValue[i] = 0;
	}
}
void TokenChannels_t::Tokenize( const char *string )
{
	SetDefaultChannels();
	if ( !string || !*string )
		return;

	if ( Q_strlen( string ) > 128 )
	{
		iActiveChannels = 1;
		bHasAnyValues = true;
		bReady = true;
		bChannelAsValue[0] = true;
	}

	while ( *string == ' ' )
		string++;
	const bool bNormalize = string && (*string == 'n' || *string == 'N' );
	if ( bNormalize )
		string++;
	while ( *string == ' ' )
		string++;

	char localString[ MAX_OPERATION_C ];
	Q_snprintf( localString, sizeof(localString), "%s\0", string );

	char *MakeUpper = localString;
	while( MakeUpper && *MakeUpper )
	{
		if ( *MakeUpper == 'e' )
			*MakeUpper = 'E';
		else if ( *MakeUpper == 'p' )
			*MakeUpper = 'P';
		else if ( *MakeUpper == 'i' )
			*MakeUpper = 'I';
		MakeUpper++;
	}

	const char *stringtest = localString;
	while (stringtest && *stringtest)
	{
		if ( ISSTRING_LITERAL( localString, stringtest ) || ISSTRING_VALUE( localString, stringtest ) )
			bHasAnyValues = true;
		stringtest++;
	}

	char *cleanupReader = localString;
	while ( *cleanupReader )
	{
		if ( !ISSTRING_VALUE( localString, cleanupReader ) &&
				*cleanupReader != ' ' &&
				!ISSTRING_OPERATOR( cleanupReader ) &&
				GetChannelNumFromChar( cleanupReader ) < 0 &&
				!ISSTRING_LITERAL( localString, cleanupReader ) &&
				!ISCHAR_BRACKET( *cleanupReader ) &&
				*cleanupReader != '$'
				)
		{
			SnipCharFromString( cleanupReader );
			//cleanupReader--;
		}
		cleanupReader++;
	}

	char cleanupString[MAX_OPERATION_C];
	cleanupReader = cleanupString;
	Q_strcpy( cleanupString, localString );
	Q_memset( localString, 0, sizeof( localString ) );

#define STEST_VALUE 0
#define STEST_OPERATOR 1
#define STEST_BRACKET_OPEN 2
#define STEST_BRACKET_CLOSE 3
#define STEST_BRACKET( x ) ( x == STEST_BRACKET_OPEN || x == STEST_BRACKET_CLOSE )

	while( *cleanupReader )
	{
		if ( *cleanupReader == ' ' )
		{
			const char *searchfront = cleanupReader;
			const char *searchback = cleanupReader;

			int priorToSpace = -1;
			int afterSpace = -1;
			while ( searchfront >= cleanupString )
			{
				if ( ISSTRING_VALUE( cleanupString, searchfront ) ||
					ISSTRING_LITERAL( cleanupString, searchfront ) || *searchfront == '$' )
					priorToSpace = STEST_VALUE;
				else if ( ISSTRING_OPERATOR( searchfront ) )
					priorToSpace = STEST_OPERATOR;
				else if ( ISCHAR_BOPEN( *searchfront ) )
					priorToSpace = STEST_BRACKET_OPEN;
				else if ( ISCHAR_BCLOSE( *searchfront ) )
					priorToSpace = STEST_BRACKET_CLOSE;
				else if ( GetChannelNumFromChar( searchfront ) >= 0 )
					break;
				if ( priorToSpace >= 0 )
					break;
				searchfront--;
			}
			while ( *searchback )
			{
				if ( ISSTRING_VALUE( cleanupString, searchback ) ||
					ISSTRING_LITERAL( cleanupString, searchback ) || *searchback == '$' )
					afterSpace = STEST_VALUE;
				else if ( ISSTRING_OPERATOR( searchback ) )
					afterSpace = STEST_OPERATOR;
				else if ( ISCHAR_BOPEN( *searchback ) )
					afterSpace = STEST_BRACKET_OPEN;
				else if ( ISCHAR_BCLOSE( *searchback ) )
					afterSpace = STEST_BRACKET_CLOSE;
				else if ( GetChannelNumFromChar( searchback ) >= 0 )
					break;
				if ( afterSpace >= 0 )
					break;
				searchback++;
			}
			//			1 * ( 2 + 1 ) ( 3 ) + 1
			//			1 ( 10 ) 0
			if (
				( !STEST_BRACKET(priorToSpace) &&
				!STEST_BRACKET(afterSpace) &&
				priorToSpace != afterSpace &&
				priorToSpace >= 0 &&
				afterSpace >= 0) ||
				priorToSpace == STEST_BRACKET_OPEN ||
				afterSpace == STEST_BRACKET_CLOSE ||
				(STEST_BRACKET(priorToSpace) && afterSpace == STEST_OPERATOR ) || //!STEST_BRACKET(afterSpace)) ||
				(STEST_BRACKET(afterSpace) && priorToSpace == STEST_OPERATOR ) //!STEST_BRACKET(priorToSpace))
				)
			{
				SnipCharFromString( cleanupReader );
				cleanupReader--;
			}
		}
		cleanupReader++;
	}

	Q_strcpy( localString, cleanupString );

	char *GlobalExpression = localString;
	while( *GlobalExpression )
	{
		if ( *GlobalExpression == '$' )
		{
			*GlobalExpression = '\0';
			GlobalExpression++;
			while ( *GlobalExpression == ' ' )
				GlobalExpression++;

			char *SetEnd = GlobalExpression;
			while ( *SetEnd )
			{
				if ( *SetEnd == '$' )
					*SetEnd = '\0';
				SetEnd++;
			}
			break;
		}
		GlobalExpression++;
	}

#if 0
	cleanupReader = localString;
	while( *cleanupReader )
	{
		if ( ISCHAR_BRACKET( *cleanupReader ) )
			SnipCharFromString( cleanupReader );
		cleanupReader++;
	}
#endif

	iActiveChannels = 0;

	if ( !bHasAnyValues && bAllowChars )
	{
		int slotWrite = 0;
		const char *walk = localString;
		while (walk && *walk && slotWrite < 4)
		{
			if ( GetChannelNumFromChar( walk ) >= 0 )
			{
				bChannelAsValue[ slotWrite ] = false;
				cChannel[ slotWrite ] = *walk;
				slotWrite++;
				iActiveChannels++;
			}
			walk++;
		}
	}
	else
	{
		SimpleTokenize _tokens;
		_tokens.Tokenize( localString );
		//CCommand args;
		//args.Tokenize( localString );
		for ( int i = 0; i < min( _tokens.Count(), 4 ); i++ )
		//for ( int i = 0; i < min( args.ArgC(), 4 ); i++ )
		{
			const char *token = _tokens.GetToken( i );
			//const char *token = args[i];
			if ( GetChannelNumFromChar( token ) >= 0 && bAllowChars )
			{
				bChannelAsValue[ i ] = false;
				cChannel[ i ] = *token;
			}
			else
			{
				bChannelAsValue[ i ] = true;

				const bool bDoGlExp = (GlobalExpression && *GlobalExpression);
				char tmp[ MAX_OPERATION_C ];
				char *target = tmp;
				if ( bDoGlExp )
				{
					*target = '(';
					target++;
				}

				Q_strcpy( target, token );

				if ( bDoGlExp )
				{
					Q_strcat( tmp, ")", MAX_OPERATION_C );
					Q_strcat( tmp, GlobalExpression, MAX_OPERATION_C );
				}
				
				flValue[ i ] = PerformSolve( tmp );
			}
			iActiveChannels++;
		}
	}

	bReady = !!iActiveChannels;
	Assert( iActiveChannels >= 0 && iActiveChannels <= 4 );
	iChannelDepth = 0;

	for ( int i = 0; i < iActiveChannels; i++ )
		if ( bChannelAsValue[ i ] )
			bHasAnyValues = true;

	for ( int i = 0; i < iActiveChannels; i++ )
	{
		iChannel[ i ] = 0;
		if ( bChannelAsValue[i] )
			continue;

		iChannel[ i ] = GetChannelNumFromChar( &cChannel[i] );
		int num = iChannel[ i ] + 1;
		iChannelDepth = max( num, iChannelDepth );
	}

// manual normalize
	if ( bNormalize )
	{
		float flVec[4];
		Q_memset( flVec, 0, sizeof(flVec) );

		for ( int i = 0; i < iActiveChannels; i++ )
			if ( bChannelAsValue[ i ] )
				flVec[ i ] = flValue[ i ];

		double vecSize = 0;
		for ( int i = 0; i < 4; i++ )
			vecSize += Sqr( flVec[i] );

		if ( vecSize == 0 )
			vecSize = 1;
		vecSize = FastSqrt( vecSize );
		vecSize = 1.0f / vecSize;

		for ( int i = 0; i < 4; i++ )
			if ( bChannelAsValue[ i ] )
				flValue[ i ] *= vecSize;
	}
}
void TokenChannels_t::PrintTargetString( char *out, int maxlen )
{
	if ( maxlen < 1 )
		return;

	Q_memset( out, 0, maxlen );
	char tmp[MAX_PATH];
	for ( int i = 0; i < iActiveChannels; i++ )
	{
		if ( bChannelAsValue[ i ] )
		{
			float frac = (flValue[i]) - ((int)flValue[i]);
			if ( frac != 0.0f )
			{
				Q_memset( tmp, 0, sizeof(tmp) );
				Q_snprintf( tmp, MAX_PATH, "%f\0", flValue[i] );
				for ( int a = (MAX_PATH-1); a >= 0; a-- )
				{
					if ( tmp[a] && tmp[a] != '0' )
						break;
					tmp[a] = '\0';
				}
			}
			else
				Q_snprintf( tmp, MAX_PATH, "%i\0", ((int)flValue[i]) );
		}
		else
			Q_snprintf( tmp, MAX_PATH, "%c\0", cChannel[i] );

		Q_strcat( out, tmp, maxlen );
		if ( i < (iActiveChannels-1) && bHasAnyValues )
			Q_strcat( out, " ", maxlen );
	}
}
void TokenChannels_t::ClearString( const char *in, char *out, int maxlen )
{
	bool bOnlyValidChannels = true;
	TokenChannels_t tmp;
	tmp.Tokenize( in );
	if ( tmp.bHasAnyValues )
		bOnlyValidChannels = false;

	int slot = 0;
	char *writer = out;
	const char *reader = in;
	while ( reader && *reader )
	{
		if ( GetChannelNumFromChar( reader ) >= 0 ||
			( !bOnlyValidChannels && *reader >= '0' && *reader <= '9' ) ||
			( !bOnlyValidChannels && *reader == ' ' ) ||
			( !bOnlyValidChannels && *reader == '.' ) )
		{
			if ( slot >= (maxlen-1) )
				break;
			*writer = *reader;
			writer++;
			slot++;
		}
		reader++;
	}
	Assert( slot < maxlen );
	*writer = '\0';

	RemoveDuplicatedSpaces( out );
	CutString( out );
}
void TokenChannels_t::RemoveDuplicatedSpaces( char *in )
{
	int activeSpaces = 0;

	char *writer = in;
	char *reader = in;
	while ( reader && *reader )
	{
		bool bWrite = true;
		if ( *reader == ' ' )
		{
			activeSpaces++;
			if ( activeSpaces >= 2 )
			{
				bWrite = false;
			}
		}
		else
			activeSpaces = 0;
		if ( bWrite )
		{
			*writer = *reader;
			writer++;
		}
		reader++;
	}
	writer--;
	while ( *writer == ' ' )
		writer--;
	writer++;
	if ( *writer )
	{
		*writer = '\0';
	}
}
void TokenChannels_t::CutString( char *in )
{
	int countedSpaces = 0;
	char *writer = in;
	while ( writer && *writer )
	{
		if ( *writer == ' ' )
		{
			countedSpaces++;
			if ( countedSpaces >= 4 )
			{
				*writer = '\0';
			}
		}
		writer++;
	}
}



SimpleTokenize::SimpleTokenize()
{
}
SimpleTokenize::~SimpleTokenize()
{
	ClearBuffer();
}
void SimpleTokenize::ClearBuffer()
{
	for ( int i = 0; i < hTokens.Count(); i++ )
		delete [] hTokens[i];
	hTokens.Purge();
}
void SimpleTokenize::Tokenize( const char *szString, const bool bAutoClear )
{
	if ( bAutoClear )
		ClearBuffer();

	char *buffer = new char[ Q_strlen( szString ) + 1 ];
	Q_strcpy( buffer, szString );

	// clear double spaces
	char *readSpaces = buffer;
	while( *readSpaces )
	{
		if ( *readSpaces == ' ' )
		{
			char *ClearSpaces = readSpaces + 1;
			while ( *ClearSpaces == ' ' )
				SnipCharFromString( ClearSpaces );
		}
		readSpaces++;
	}

	// tokenize
	const char *curToken = buffer;
	char *searchBreaks = buffer;
	while ( true ) //*searchBreaks )
	{
		const bool bEnd = !*searchBreaks;

		if ( *searchBreaks == ' ' || bEnd )
		{
			*searchBreaks = '\0';
			int len = Q_strlen( curToken );
			if ( len )
			{
				char *token = new char[ len + 1 ];
				Q_strcpy( token, curToken );
				hTokens.AddToTail( token );
				curToken = searchBreaks + 1;
			}
			if ( bEnd )
				break;
		}
		searchBreaks++;
	}

	delete [] buffer;
}
int SimpleTokenize::Count()
{
	return hTokens.Count();
}
const char* SimpleTokenize::GetToken( int i )
{
	Assert( i >= 0 && i < hTokens.Count() );
	return hTokens[ i ];
}