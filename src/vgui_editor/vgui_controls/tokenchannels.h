#ifndef TOKENCHANNELS_H
#define TOKENCHANNELS_H

#include "vgui_controls/utilities.h"

struct SimpleTokenize
{
public:
	SimpleTokenize();
	~SimpleTokenize();

	void Tokenize( const char *szString, const bool bAutoClear = true );
	void ClearBuffer();

	int Count();
	const char* GetToken( int i );

	char *operator[](int i)
	{
		Assert( i >= 0 && i < Count() );
		return hTokens[i];
	}
private:
	CUtlVector< char* > hTokens;
};


struct TokenChannels_t
{
	TokenChannels_t();

	void SetDefaultChannels();

	static void RemoveDuplicatedSpaces( char *in );
	static void CutString( char *in );
	static void ClearString( const char *in, char *out, int maxlen );
	void PrintTargetString( char *out, int maxlen );

	void Tokenize( const char *string );
	const bool IsActive() const;

	bool bHasAnyValues;
	bool bChannelAsValue[4];

	char cChannel[4];
	int iChannel[4];

	float flValue[4];
	int iActiveChannels;
	int iChannelDepth;

	bool bAllowChars;
private:
	bool bReady;

	float PerformSolve( const char *start );
};

#endif