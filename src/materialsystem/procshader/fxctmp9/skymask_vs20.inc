#include "shaderlib/cshader.h"
class skymask_vs20_Static_Index
{
private:
	int m_nDRAWWHITE;
#ifdef _DEBUG
	bool m_bDRAWWHITE;
#endif
public:
	void SetDRAWWHITE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDRAWWHITE = i;
#ifdef _DEBUG
		m_bDRAWWHITE = true;
#endif
	}
	void SetDRAWWHITE( bool i )
	{
		m_nDRAWWHITE = i ? 1 : 0;
#ifdef _DEBUG
		m_bDRAWWHITE = true;
#endif
	}
private:
	int m_nCOMBINE;
#ifdef _DEBUG
	bool m_bCOMBINE;
#endif
public:
	void SetCOMBINE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOMBINE = i;
#ifdef _DEBUG
		m_bCOMBINE = true;
#endif
	}
	void SetCOMBINE( bool i )
	{
		m_nCOMBINE = i ? 1 : 0;
#ifdef _DEBUG
		m_bCOMBINE = true;
#endif
	}
public:
	skymask_vs20_Static_Index( )
	{
#ifdef _DEBUG
		m_bDRAWWHITE = false;
#endif // _DEBUG
		m_nDRAWWHITE = 0;
#ifdef _DEBUG
		m_bCOMBINE = false;
#endif // _DEBUG
		m_nCOMBINE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bDRAWWHITE && m_bCOMBINE;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nDRAWWHITE ) + ( 2 * m_nCOMBINE ) + 0;
	}
};
#define shaderStaticTest_skymask_vs20 vsh_forgot_to_set_static_DRAWWHITE + vsh_forgot_to_set_static_COMBINE + 0
class skymask_vs20_Dynamic_Index
{
public:
	skymask_vs20_Dynamic_Index()
	{
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
#endif // _DEBUG
		return 0;
	}
};
#define shaderDynamicTest_skymask_vs20 0
