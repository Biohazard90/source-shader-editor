#ifndef GENERAL_UTILITIES
#define GENERAL_UTILITIES

char *V_StripFirstDir (char *path);
void V_IsolateFirstDir (char *path);


#define Q_StripFirstDir			V_StripFirstDir
#define Q_IsolateFirstDir		V_IsolateFirstDir

#ifdef SHADER_EDITOR_DLL_2006

#include "exportedtier1/byteswap.h"
#include "exportedtier1/characterset.h"

template < typename T >
class CPlainAutoPtr
{
public:
	explicit CPlainAutoPtr( T *p = NULL )		: m_p( p ) {}
	~CPlainAutoPtr( void )						{ Delete(); }

public:
	void Delete( void )							{ delete Detach(); }

private:	// Disallow copying, use Detach() instead to avoid ambiguity
	CPlainAutoPtr( CPlainAutoPtr const &x );
	CPlainAutoPtr & operator = ( CPlainAutoPtr const &x );

public:
	void Attach( T *p )							{ m_p = p; }
	T * Detach( void )							{ T * p( m_p ); m_p = NULL; return p; }

public:
	bool IsValid( void ) const					{ return m_p != NULL; }
	T * Get( void ) const						{ return m_p; }
	T * operator -> ( void ) const				{ return Get(); }
	T & operator *  ( void ) const				{ return *Get(); }

private:
	T * m_p;
};

void GetMatricesForView( CViewSetup &setup, VMatrix &view, VMatrix &proj, VMatrix &viewproj );

void InitLightDirectional( LightDesc_t &l, Vector dir, Vector color );

#endif


class CUtlBufferEditor
{
public:
	enum SeekType_t
	{
		SEEK_HEAD = 0,
		SEEK_CURRENT,
		SEEK_TAIL
	};

	// flags
	enum BufferFlags_t
	{
		TEXT_BUFFER = 0x1,			// Describes how get + put work (as strings, or binary)
		EXTERNAL_GROWABLE = 0x2,	// This is used w/ external buffers and causes the utlbuf to switch to reallocatable memory if an overflow happens when Putting.
		CONTAINS_CRLF = 0x4,		// For text buffers only, does this contain \n or \n\r?
		READ_ONLY = 0x8,			// For external buffers; prevents null termination from happening.
		AUTO_TABS_DISABLED = 0x10,	// Used to disable/enable push/pop tabs
		DIRECTIVE_TEST_ENABLED = 0x20,	// Used to disable/enable push/pop tabs
	};

	// Overflow functions when a get or put overflows
	typedef bool (CUtlBufferEditor::*UtlBufferOverflowFunc_t)( int nSize );

	void WriteToBuffer( CUtlBuffer &buf );

	// Constructors for growable + external buffers for serialization/unserialization
	CUtlBufferEditor( int growSize = 0, int initSize = 0, int nFlags = 0 );
	CUtlBufferEditor( const void* pBuffer, int size, int nFlags = 0 );
	// This one isn't actually defined so that we catch contructors that are trying to pass a bool in as the third param.
	CUtlBufferEditor( const void *pBuffer, int size, bool crap );

	unsigned char	GetFlags() const;

	// NOTE: This will assert if you attempt to recast it in a way that
	// is not compatible. The only valid conversion is binary-> text w/CRLF
	void			SetBufferType( bool bIsText, bool bContainsCRLF );

	// Makes sure we've got at least this much memory
	void			EnsureCapacity( int num );

	// Attaches the buffer to external memory....
	void			SetExternalBuffer( void* pMemory, int nSize, int nInitialPut, int nFlags = 0 );
	bool			IsExternallyAllocated() const;
	void			AssumeMemory( void *pMemory, int nSize, int nInitialPut, int nFlags = 0 );

	FORCEINLINE void ActivateByteSwappingIfBigEndian( void )
	{
		if ( IsX360() )
			ActivateByteSwapping( true );
	}


	// Controls endian-ness of binary utlbufs - default matches the current platform
	void			ActivateByteSwapping( bool bActivate );
	void			SetBigEndian( bool bigEndian );
	bool			IsBigEndian( void );

	// Resets the buffer; but doesn't free memory
	void			Clear();

	// Clears out the buffer; frees memory
	void			Purge();

	// Read stuff out.
	// Binary mode: it'll just read the bits directly in, and characters will be
	//		read for strings until a null character is reached.
	// Text mode: it'll parse the file, turning text #s into real numbers.
	//		GetString will read a string until a space is reached
	char			GetChar( );
	unsigned char	GetUnsignedChar( );
	short			GetShort( );
	unsigned short	GetUnsignedShort( );
	int				GetInt( );
	int				GetIntHex( );
	unsigned int	GetUnsignedInt( );
	float			GetFloat( );
	double			GetDouble( );
	void			GetString( char* pString, int nMaxChars = 0 );
	void			Get( void* pMem, int size );
	void			GetLine( char* pLine, int nMaxChars = 0 );

	// Used for getting objects that have a byteswap datadesc defined
	template <typename T> void GetObjects( T *dest, int count = 1 );

	// This will get at least 1 byte and up to nSize bytes. 
	// It will return the number of bytes actually read.
	int				GetUpTo( void *pMem, int nSize );

	// This version of GetString converts \" to \\ and " to \, etc.
	// It also reads a " at the beginning and end of the string
	void			GetDelimitedString( CUtlCharConversion *pConv, char *pString, int nMaxChars = 0 );
	char			GetDelimitedChar( CUtlCharConversion *pConv );

	// This will return the # of characters of the string about to be read out
	// NOTE: The count will *include* the terminating 0!!
	// In binary mode, it's the number of characters until the next 0
	// In text mode, it's the number of characters until the next space.
	int				PeekStringLength();

	// This version of PeekStringLength converts \" to \\ and " to \, etc.
	// It also reads a " at the beginning and end of the string
	// NOTE: The count will *include* the terminating 0!!
	// In binary mode, it's the number of characters until the next 0
	// In text mode, it's the number of characters between "s (checking for \")
	// Specifying false for bActualSize will return the pre-translated number of characters
	// including the delimiters and the escape characters. So, \n counts as 2 characters when bActualSize == false
	// and only 1 character when bActualSize == true
	int				PeekDelimitedStringLength( CUtlCharConversion *pConv, bool bActualSize = true );

	// Just like scanf, but doesn't work in binary mode
	int				Scanf( const char* pFmt, ... );
	int				VaScanf( const char* pFmt, va_list list );

	// Eats white space, advances Get index
	void			EatWhiteSpace();

	// Eats C++ style comments
	bool			EatCPPComment();

	// (For text buffers only)
	// Parse a token from the buffer:
	// Grab all text that lies between a starting delimiter + ending delimiter
	// (skipping whitespace that leads + trails both delimiters).
	// If successful, the get index is advanced and the function returns true,
	// otherwise the index is not advanced and the function returns false.
	bool			ParseToken( const char *pStartingDelim, const char *pEndingDelim, char* pString, int nMaxLen );

	// Advance the get index until after the particular string is found
	// Do not eat whitespace before starting. Return false if it failed
	// String test is case-insensitive.
	bool			GetToken( const char *pToken );

	// Parses the next token, given a set of character breaks to stop at
	// Returns the length of the token parsed in bytes (-1 if none parsed)
	int				ParseToken( characterset_t *pBreaks, char *pTokenBuf, int nMaxLen, bool bParseComments = true );

	// Write stuff in
	// Binary mode: it'll just write the bits directly in, and strings will be
	//		written with a null terminating character
	// Text mode: it'll convert the numbers to text versions
	//		PutString will not write a terminating character
	void			PutChar( char c );
	void			PutUnsignedChar( unsigned char uc );
	void			PutShort( short s );
	void			PutUnsignedShort( unsigned short us );
	void			PutInt( int i );
	void			PutUnsignedInt( unsigned int u );
	void			PutFloat( float f );
	void			PutDouble( double d );
	void			PutString( const char* pString );
	void			Put( const void* pMem, int size );

	// Used for putting objects that have a byteswap datadesc defined
	template <typename T> void PutObjects( T *src, int count = 1 );

	// This version of PutString converts \ to \\ and " to \", etc.
	// It also places " at the beginning and end of the string
	void			PutDelimitedString( CUtlCharConversion *pConv, const char *pString );
	void			PutDelimitedChar( CUtlCharConversion *pConv, char c );

	// Just like printf, writes a terminating zero in binary mode
	void			Printf( const char* pFmt, ... );
	void			VaPrintf( const char* pFmt, va_list list );

	// What am I writing (put)/reading (get)?
	void* PeekPut( int offset = 0 );
	const void* PeekGet( int offset = 0 ) const;
	const void* PeekGet( int nMaxSize, int nOffset );

	// Where am I writing (put)/reading (get)?
	int TellPut( ) const;
	int TellGet( ) const;

	// What's the most I've ever written?
	int TellMaxPut( ) const;

	// How many bytes remain to be read?
	// NOTE: This is not accurate for streaming text files; it overshoots
	int GetBytesRemaining() const;

	// Change where I'm writing (put)/reading (get)
	void SeekPut( SeekType_t type, int offset );
	void SeekGet( SeekType_t type, int offset );

	// Buffer base
	const void* Base() const;
	void* Base();

	// memory allocation size, does *not* reflect size written or read,
	//	use TellPut or TellGet for that
	int Size() const;

	// Am I a text buffer?
	bool IsText() const;

	// Can I grow if I'm externally allocated?
	bool IsGrowable() const;

	// Am I valid? (overflow or underflow error), Once invalid it stays invalid
	bool IsValid() const;

	// Do I contain carriage return/linefeeds? 
	bool ContainsCRLF() const;

	// Am I read-only
	bool IsReadOnly() const;

	// Converts a buffer from a CRLF buffer to a CR buffer (and back)
	// Returns false if no conversion was necessary (and outBuf is left untouched)
	// If the conversion occurs, outBuf will be cleared.
	bool ConvertCRLF( CUtlBufferEditor &outBuf );

	// Push/pop pretty-printing tabs
	void PushTab();
	void PopTab();

	// Temporarily disables pretty print
	void EnableTabs( bool bEnable );

	void EnableDirectives( bool bEnable );

protected:
	// error flags
	enum
	{
		PUT_OVERFLOW = 0x1,
		GET_OVERFLOW = 0x2,
		MAX_ERROR_FLAG = GET_OVERFLOW,
	};

	void SetOverflowFuncs( UtlBufferOverflowFunc_t getFunc, UtlBufferOverflowFunc_t putFunc );

	bool OnPutOverflow( int nSize );
	bool OnGetOverflow( int nSize );

protected:
	// Checks if a get/put is ok
	bool CheckPut( int size );
	bool CheckGet( int size );

	void AddNullTermination( );

	// Methods to help with pretty-printing
	bool WasLastCharacterCR();
	bool IsDirectiveLine( const char *stringIn );
	void PutTabs();

	// Help with delimited stuff
	char GetDelimitedCharInternal( CUtlCharConversion *pConv );
	void PutDelimitedCharInternal( CUtlCharConversion *pConv, char c );

	// Default overflow funcs
	bool PutOverflow( int nSize );
	bool GetOverflow( int nSize );

	// Does the next bytes of the buffer match a pattern?
	bool PeekStringMatch( int nOffset, const char *pString, int nLen );

	// Peek size of line to come, check memory bound
	int	PeekLineLength();

	// How much whitespace should I skip?
	int PeekWhiteSpace( int nOffset );

	// Checks if a peek get is ok
	bool CheckPeekGet( int nOffset, int nSize );

	// Call this to peek arbitrarily long into memory. It doesn't fail unless
	// it can't read *anything* new
	bool CheckArbitraryPeekGet( int nOffset, int &nIncrement );

	template <typename T> void GetType( T& dest, const char *pszFmt );
	template <typename T> void GetTypeBin( T& dest );
	template <typename T> void GetObject( T *src );

	template <typename T> void PutType( T src, const char *pszFmt );
	template <typename T> void PutTypeBin( T src );
	template <typename T> void PutObject( T *src );

	CUtlMemory<unsigned char> m_Memory;
	int m_Get;
	int m_Put;

	unsigned char m_Error;
	unsigned char m_Flags;
	unsigned char m_Reserved;
#if defined( _X360 )
	unsigned char pad;
#endif

	int m_nTab;
	int m_nMaxPut;
	int m_nOffset;

	UtlBufferOverflowFunc_t m_GetOverflowFunc;
	UtlBufferOverflowFunc_t m_PutOverflowFunc;

	CByteswap	m_Byteswap;
};


// Stream style output operators for CUtlBufferEditor
inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, char v )
{
	b.PutChar( v );
	return b;
}

inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, unsigned char v )
{
	b.PutUnsignedChar( v );
	return b;
}

inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, short v )
{
	b.PutShort( v );
	return b;
}

inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, unsigned short v )
{
	b.PutUnsignedShort( v );
	return b;
}

inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, int v )
{
	b.PutInt( v );
	return b;
}

inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, unsigned int v )
{
	b.PutUnsignedInt( v );
	return b;
}

inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, float v )
{
	b.PutFloat( v );
	return b;
}

inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, double v )
{
	b.PutDouble( v );
	return b;
}

inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, const char *pv )
{
	b.PutString( pv );
	return b;
}

inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, const Vector &v )
{
	b << v.x << " " << v.y << " " << v.z;
	return b;
}

inline CUtlBufferEditor &operator<<( CUtlBufferEditor &b, const Vector2D &v )
{
	b << v.x << " " << v.y;
	return b;
}



//-----------------------------------------------------------------------------
// Where am I reading?
//-----------------------------------------------------------------------------
inline int CUtlBufferEditor::TellGet( ) const
{
	return m_Get;
}


//-----------------------------------------------------------------------------
// How many bytes remain to be read?
//-----------------------------------------------------------------------------
inline int CUtlBufferEditor::GetBytesRemaining() const
{
	return m_nMaxPut - TellGet();
}


//-----------------------------------------------------------------------------
// What am I reading?
//-----------------------------------------------------------------------------
inline const void* CUtlBufferEditor::PeekGet( int offset ) const
{
	return &m_Memory[ m_Get + offset - m_nOffset ];
}


//-----------------------------------------------------------------------------
// Unserialization
//-----------------------------------------------------------------------------

template <typename T> 
inline void CUtlBufferEditor::GetObject( T *dest )
{
	if ( CheckGet( sizeof(T) ) )
	{
		if ( !m_Byteswap.IsSwappingBytes() || ( sizeof( T ) == 1 ) )
		{
			*dest = *(T *)PeekGet();
		}
		else
		{
			m_Byteswap.SwapFieldsToTargetEndian<T>( dest, (T*)PeekGet() );
		}
		m_Get += sizeof(T);	
	}
	else
	{
		Q_memset( &dest, 0, sizeof(T) );
	}
}


template <typename T> 
inline void CUtlBufferEditor::GetObjects( T *dest, int count )
{
	for ( int i = 0; i < count; ++i, ++dest )
	{
		GetObject<T>( dest );
	}
}


template <typename T> 
inline void CUtlBufferEditor::GetTypeBin( T &dest )
{
	if ( CheckGet( sizeof(T) ) )
	{
		if ( !m_Byteswap.IsSwappingBytes() || ( sizeof( T ) == 1 ) )
		{
			dest = *(T *)PeekGet();
		}
		else
		{
			m_Byteswap.SwapBufferToTargetEndian<T>( &dest, (T*)PeekGet() );
		}
		m_Get += sizeof(T);	
	}		
	else
	{
		dest = 0;
	}					
}

template <>
inline void CUtlBufferEditor::GetTypeBin< float >( float &dest )
{
	if ( CheckGet( sizeof( float ) ) )
	{
		unsigned int pData = (unsigned int)PeekGet();
		if ( IsX360() && ( pData & 0x03 ) )
		{
			// handle unaligned read
			((unsigned char*)&dest)[0] = ((unsigned char*)pData)[0];
			((unsigned char*)&dest)[1] = ((unsigned char*)pData)[1];
			((unsigned char*)&dest)[2] = ((unsigned char*)pData)[2];
			((unsigned char*)&dest)[3] = ((unsigned char*)pData)[3];
		}
		else
		{
			// aligned read
			dest = *(float *)pData;
		}
		if ( m_Byteswap.IsSwappingBytes() )
		{
			m_Byteswap.SwapBufferToTargetEndian< float >( &dest, &dest );
		}
		m_Get += sizeof( float );	
	}		
	else
	{
		dest = 0;
	}					
}

template <typename T> 
inline void CUtlBufferEditor::GetType( T &dest, const char *pszFmt )
{
	if (!IsText())
	{
		GetTypeBin( dest );
	}
	else
	{
		dest = 0;
		Scanf( pszFmt, &dest );
	}
}

inline char CUtlBufferEditor::GetChar( )
{
	char c;
	GetType( c, "%c" );
	return c;
}

inline unsigned char CUtlBufferEditor::GetUnsignedChar( )
{
	unsigned char c;
	GetType( c, "%u" );
	return c;
}

inline short CUtlBufferEditor::GetShort( )
{
	short s;
	GetType( s, "%d" );
	return s;
}

inline unsigned short CUtlBufferEditor::GetUnsignedShort( )
{
	unsigned short s;
	GetType( s, "%u" );
	return s;
}

inline int CUtlBufferEditor::GetInt( )
{
	int i;
	GetType( i, "%d" );
	return i;
}

inline int CUtlBufferEditor::GetIntHex( )
{
	int i;
	GetType( i, "%x" );
	return i;
}

inline unsigned int CUtlBufferEditor::GetUnsignedInt( )
{
	unsigned int u;
	GetType( u, "%u" );
	return u;
}

inline float CUtlBufferEditor::GetFloat( )
{
	float f;
	GetType( f, "%f" );
	return f;
}

inline double CUtlBufferEditor::GetDouble( )
{
	double d;
	GetType( d, "%f" );
	return d;
}


//-----------------------------------------------------------------------------
// Where am I writing?
//-----------------------------------------------------------------------------
inline unsigned char CUtlBufferEditor::GetFlags() const
{ 
	return m_Flags; 
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
inline bool CUtlBufferEditor::IsExternallyAllocated() const
{ 
	return m_Memory.IsExternallyAllocated();
}

	
//-----------------------------------------------------------------------------
// Where am I writing?
//-----------------------------------------------------------------------------
inline int CUtlBufferEditor::TellPut( ) const
{
	return m_Put;
}


//-----------------------------------------------------------------------------
// What's the most I've ever written?
//-----------------------------------------------------------------------------
inline int CUtlBufferEditor::TellMaxPut( ) const
{
	return m_nMaxPut;
}


//-----------------------------------------------------------------------------
// What am I reading?
//-----------------------------------------------------------------------------
inline void* CUtlBufferEditor::PeekPut( int offset )
{
	return &m_Memory[m_Put + offset - m_nOffset];
}


//-----------------------------------------------------------------------------
// Various put methods
//-----------------------------------------------------------------------------

template <typename T> 
inline void CUtlBufferEditor::PutObject( T *src )
{
	if ( CheckPut( sizeof(T) ) )
	{
		if ( !m_Byteswap.IsSwappingBytes() || ( sizeof( T ) == 1 ) )
		{
			*(T *)PeekPut() = *src;
		}
		else
		{
			m_Byteswap.SwapFieldsToTargetEndian<T>( (T*)PeekPut(), src );
		}
		m_Put += sizeof(T);
		AddNullTermination();
	}
}


template <typename T> 
inline void CUtlBufferEditor::PutObjects( T *src, int count )
{
	for ( int i = 0; i < count; ++i, ++src )
	{
		PutObject<T>( src );
	}
}


template <typename T> 
inline void CUtlBufferEditor::PutTypeBin( T src )
{
	if ( CheckPut( sizeof(T) ) )
	{
		if ( !m_Byteswap.IsSwappingBytes() || ( sizeof( T ) == 1 ) )
		{
			*(T *)PeekPut() = src;
		}
		else
		{
			m_Byteswap.SwapBufferToTargetEndian<T>( (T*)PeekPut(), &src );
		}
		m_Put += sizeof(T);
		AddNullTermination();
	}
}

template <typename T> 
inline void CUtlBufferEditor::PutType( T src, const char *pszFmt )
{
	if (!IsText())
	{
		PutTypeBin( src );
	}
	else
	{
		Printf( pszFmt, src );
	}
}

//-----------------------------------------------------------------------------
// Methods to help with pretty-printing
//-----------------------------------------------------------------------------
inline bool CUtlBufferEditor::WasLastCharacterCR()
{
	if ( !IsText() || (TellPut() == 0) )
		return false;
	return ( *( const char * )PeekPut( -1 ) == '\n' );
}
inline bool CUtlBufferEditor::IsDirectiveLine( const char *stringIn )
{
	if ( !(m_Flags & DIRECTIVE_TEST_ENABLED) )
		return false;

	const char *walk = stringIn;
	while ( walk && *walk )
	{
		if ( *walk == '#' )
			return true;
		if ( *walk != '\t' &&
			*walk != ' ' )
			break;
		walk++;
	}
	return false;
}
inline void CUtlBufferEditor::PutTabs()
{
	int nTabCount = ( m_Flags & AUTO_TABS_DISABLED ) ? 0 : m_nTab;
	for (int i = nTabCount; --i >= 0; )
	{
		PutTypeBin<char>( '\t' );
	}
}


//-----------------------------------------------------------------------------
// Push/pop pretty-printing tabs
//-----------------------------------------------------------------------------
inline void CUtlBufferEditor::PushTab( )
{
	++m_nTab;
}

inline void CUtlBufferEditor::PopTab()
{
	if ( --m_nTab < 0 )
	{
		m_nTab = 0;
	}
}


//-----------------------------------------------------------------------------
// Temporarily disables pretty print
//-----------------------------------------------------------------------------
inline void CUtlBufferEditor::EnableTabs( bool bEnable )
{
	if ( bEnable )
	{
		m_Flags &= ~AUTO_TABS_DISABLED;
	}
	else
	{
		m_Flags |= AUTO_TABS_DISABLED; 
	}
}
inline void CUtlBufferEditor::EnableDirectives( bool bEnable )
{
	if ( bEnable )
		m_Flags |= DIRECTIVE_TEST_ENABLED;
	else
		m_Flags &= ~DIRECTIVE_TEST_ENABLED;
}

inline void CUtlBufferEditor::PutChar( char c )
{
	if ( WasLastCharacterCR() )
	{
		PutTabs();
	}

	PutTypeBin( c );
}

inline void CUtlBufferEditor::PutUnsignedChar( unsigned char c )
{
	PutType( c, "%u" );
}

inline void  CUtlBufferEditor::PutShort( short s )
{
	PutType( s, "%d" );
}

inline void CUtlBufferEditor::PutUnsignedShort( unsigned short s )
{
	PutType( s, "%u" );
}

inline void CUtlBufferEditor::PutInt( int i )
{
	PutType( i, "%d" );
}

inline void CUtlBufferEditor::PutUnsignedInt( unsigned int u )
{
	PutType( u, "%u" );
}

inline void CUtlBufferEditor::PutFloat( float f )
{
	PutType( f, "%f" );
}

inline void CUtlBufferEditor::PutDouble( double d )
{
	PutType( d, "%f" );
}


//-----------------------------------------------------------------------------
// Am I a text buffer?
//-----------------------------------------------------------------------------
inline bool CUtlBufferEditor::IsText() const 
{ 
	return (m_Flags & TEXT_BUFFER) != 0; 
}


//-----------------------------------------------------------------------------
// Can I grow if I'm externally allocated?
//-----------------------------------------------------------------------------
inline bool CUtlBufferEditor::IsGrowable() const 
{ 
	return (m_Flags & EXTERNAL_GROWABLE) != 0; 
}


//-----------------------------------------------------------------------------
// Am I valid? (overflow or underflow error), Once invalid it stays invalid
//-----------------------------------------------------------------------------
inline bool CUtlBufferEditor::IsValid() const 
{ 
	return m_Error == 0; 
}


//-----------------------------------------------------------------------------
// Do I contain carriage return/linefeeds? 
//-----------------------------------------------------------------------------
inline bool CUtlBufferEditor::ContainsCRLF() const 
{ 
	return IsText() && ((m_Flags & CONTAINS_CRLF) != 0); 
} 


//-----------------------------------------------------------------------------
// Am I read-only
//-----------------------------------------------------------------------------
inline bool CUtlBufferEditor::IsReadOnly() const
{
	return (m_Flags & READ_ONLY) != 0; 
}


//-----------------------------------------------------------------------------
// Buffer base and size
//-----------------------------------------------------------------------------
inline const void* CUtlBufferEditor::Base() const	
{ 
	return m_Memory.Base(); 
}

inline void* CUtlBufferEditor::Base()
{
	return m_Memory.Base(); 
}

inline int CUtlBufferEditor::Size() const			
{ 
	return m_Memory.NumAllocated(); 
}


//-----------------------------------------------------------------------------
// Clears out the buffer; frees memory
//-----------------------------------------------------------------------------
inline void CUtlBufferEditor::Clear()
{
	m_Get = 0;
	m_Put = 0;
	m_Error = 0;
	m_nOffset = 0;
	m_nMaxPut = -1;
	AddNullTermination();
}

inline void CUtlBufferEditor::Purge()
{
	m_Get = 0;
	m_Put = 0;
	m_nOffset = 0;
	m_nMaxPut = 0;
	m_Error = 0;
	m_Memory.Purge();
}

#endif