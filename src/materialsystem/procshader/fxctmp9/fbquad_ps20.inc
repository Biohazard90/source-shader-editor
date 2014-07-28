#include "shaderlib/cshader.h"
class fbquad_ps20_Static_Index
{
private:
	int m_nCOMBINEMODE;
#ifdef _DEBUG
	bool m_bCOMBINEMODE;
#endif
public:
	void SetCOMBINEMODE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOMBINEMODE = i;
#ifdef _DEBUG
		m_bCOMBINEMODE = true;
#endif
	}
	void SetCOMBINEMODE( bool i )
	{
		m_nCOMBINEMODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bCOMBINEMODE = true;
#endif
	}
public:
	fbquad_ps20_Static_Index( )
	{
#ifdef _DEBUG
		m_bCOMBINEMODE = false;
#endif // _DEBUG
		m_nCOMBINEMODE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bCOMBINEMODE;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nCOMBINEMODE ) + 0;
	}
};
#define shaderStaticTest_fbquad_ps20 psh_forgot_to_set_static_COMBINEMODE + 0
class fbquad_ps20_Dynamic_Index
{
public:
	fbquad_ps20_Dynamic_Index()
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
#define shaderDynamicTest_fbquad_ps20 0
