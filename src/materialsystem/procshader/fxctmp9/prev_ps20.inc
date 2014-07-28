#include "shaderlib/cshader.h"
class prev_ps20_Static_Index
{
public:
	prev_ps20_Static_Index( )
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
#define shaderStaticTest_prev_ps20 0
class prev_ps20_Dynamic_Index
{
private:
	int m_nISCUBEMAP;
#ifdef _DEBUG
	bool m_bISCUBEMAP;
#endif
public:
	void SetISCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nISCUBEMAP = i;
#ifdef _DEBUG
		m_bISCUBEMAP = true;
#endif
	}
	void SetISCUBEMAP( bool i )
	{
		m_nISCUBEMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bISCUBEMAP = true;
#endif
	}
public:
	prev_ps20_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bISCUBEMAP = false;
#endif // _DEBUG
		m_nISCUBEMAP = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bISCUBEMAP;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nISCUBEMAP ) + 0;
	}
};
#define shaderDynamicTest_prev_ps20 psh_forgot_to_set_dynamic_ISCUBEMAP + 0
