#include "shaderlib/cshader.h"
class psin_ps20_Static_Index
{
public:
	psin_ps20_Static_Index( )
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
#define shaderStaticTest_psin_ps20 0
class psin_ps20_Dynamic_Index
{
private:
	int m_nOPERATION;
#ifdef _DEBUG
	bool m_bOPERATION;
#endif
public:
	void SetOPERATION( int i )
	{
		Assert( i >= 0 && i <= 9 );
		m_nOPERATION = i;
#ifdef _DEBUG
		m_bOPERATION = true;
#endif
	}
	void SetOPERATION( bool i )
	{
		m_nOPERATION = i ? 1 : 0;
#ifdef _DEBUG
		m_bOPERATION = true;
#endif
	}
public:
	psin_ps20_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bOPERATION = false;
#endif // _DEBUG
		m_nOPERATION = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bOPERATION;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nOPERATION ) + 0;
	}
};
#define shaderDynamicTest_psin_ps20 psh_forgot_to_set_dynamic_OPERATION + 0
