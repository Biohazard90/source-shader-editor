
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_Constant::CHLSL_Solver_Constant( HNODE nodeidx  ) : CHLSL_SolverBase( nodeidx )
{
	m_ConstantData.SetDefaultChannels();
}
CHLSL_Solver_Constant::CHLSL_Solver_Constant( const CHLSL_Solver_Constant& o ) : CHLSL_SolverBase( o )
{
	m_ConstantData = o.m_ConstantData;
}
CHLSL_SolverBase *CHLSL_Solver_Constant::Copy()
{
	return new CHLSL_Solver_Constant( *this );
}
void CHLSL_Solver_Constant::SetComponentInfo( const TokenChannels_t &info )
{
	m_ConstantData = info;
}

void CHLSL_Solver_Constant::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];

	Assert( m_ConstantData.iActiveChannels );

	char finalName[MAXTARGC];
	finalName[0] = '\0';
	Q_strcat( finalName, ::GetVarTypeNameCode( m_ConstantData.iActiveChannels-1 ), MAXTARGC );
	Q_strcat( finalName, "( ", MAXTARGC );
	for ( int i = 0; i < m_ConstantData.iActiveChannels; i++ )
	{
		Assert( m_ConstantData.bChannelAsValue[ i ] );

		Q_snprintf( tmp, MAXTARGC, "%f", m_ConstantData.flValue[ i ] );
		Q_strcat( finalName, tmp, MAXTARGC );

		if ( i < (m_ConstantData.iActiveChannels-1) )
			Q_strcat( finalName, ", ", MAXTARGC );
	}
	Q_strcat( finalName, " )", MAXTARGC );
	target->SetName( finalName, true );
}

void CHLSL_Solver_Constant::Render( Preview2DContext &c )
{
	float info_0[4] = { 0, 0, 0, 0 };
	for ( int i = 0; i < m_ConstantData.iActiveChannels; i++ )
	{
		info_0[i] = m_ConstantData.flValue[i];
	}

	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( info_0, 4 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}



int GetCharWeight( const char &c )
{
	if ( c == '\0' )
		return 0;
	if ( c >= '0' &&
		c <= '9' )
		return ((int)c) - ((int)'0');
	else if ( c >= 'a' &&
		c <= 'f' )
		return ((int)c) - ((int)'a');
	else if ( c >= 'A' &&
		c <= 'F' )
		return ((int)c) - ((int)'A');
	else Assert(0);
	return 0;
}
char RemapCharWeight( const int weight )
{
	if ( weight < 10 )
		return '0' + weight;
	else if ( weight < 34 )
		return 'a' + (weight-10);
	else
		return 'A' + (weight-34);
}
void CHLSL_Solver_Random::GetHash( char *out, int maxlen )
{
	if ( maxlen < 1 )
		return;
	unsigned int *var_a = (unsigned int*)&m_flValue_Min;
	unsigned int *var_b = (unsigned int*)&m_flValue_Max;

	char tmp[MAX_PATH];
	char tmp2[MAX_PATH];
	Q_memset( tmp, 0, sizeof( tmp ) );
	Q_memset( tmp2, 0, sizeof( tmp2 ) );
	Q_snprintf( tmp, MAX_PATH, "%x", *var_a );
	Q_snprintf( tmp2, MAX_PATH, "%x", *var_b );

	int strlen = min( max( Q_strlen( tmp ), Q_strlen( tmp2 ) ), (maxlen-1) );
	int written = 0;
	for ( int i = 0; i < strlen; i++ )
	{
		int c_a = GetCharWeight(tmp[i]);
		int c_b = GetCharWeight(tmp2[i]);
		c_b *= 2;
		int writeWeight = c_a + c_b;
		out[written] = RemapCharWeight( writeWeight );
		written++;
	}
	out[written] = '\0';
}
void CHLSL_Solver_Random::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];

	//char _hash[MAX_PATH];
	//GetHash( _hash, MAX_PATH );
	//Q_snprintf( tmp, MAXTARGC, "g_cData_Random%s", _hash );
	HNODE nodeIndex = GetData().iNodeIndex;
	Q_snprintf( tmp, MAXTARGC, "g_cData_Random%u", nodeIndex );

	target->SetName( tmp, true );
}
void CHLSL_Solver_Random::Render( Preview2DContext &c )
{
	float info_0[4] = { 0, 0, 0, 0 };
	for ( int i = 0; i <= m_iNumComp; i++ )
		info_0[i] = RandomFloat( m_flValue_Min, m_flValue_Max );

	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( info_0, 4 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}
void CHLSL_Solver_Random::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleEnvConstant *ec = new SimpleEnvConstant();
	ec->iEnvC_ID = HLSLENV_SMART_RANDOM_FLOAT;
	ec->iHLSLRegister = -1;
	ec->flSmartDefaultValues[0] = m_flValue_Min;
	ec->flSmartDefaultValues[1] = m_flValue_Max;
	ec->iSmartNumComps = m_iNumComp;

	char tmp[MAX_PATH];
	//char _hash[MAX_PATH];
	//GetHash( _hash, MAX_PATH );
	//Q_snprintf( tmp, MAXTARGC, "Random%s", _hash );
	HNODE nodeIndex = GetData().iNodeIndex;
	Q_snprintf( tmp, MAXTARGC, "Random%u", nodeIndex );

	ec->szSmartHelper = new char[ Q_strlen(tmp) + 1 ];
	Q_strcpy( ec->szSmartHelper, tmp );

	List.hList_EConstants.AddToTail( ec );
}







void CHLSL_Solver_BumpBasis::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );

	Assert( m_iIndex >= 0 && m_iIndex <= 2 );

	char finalName[MAXTARGC];
	Q_snprintf( finalName, MAXTARGC, "bumpBasis[%i]", m_iIndex );

	target->SetName( finalName, true );
}

void CHLSL_Solver_BumpBasis::Render( Preview2DContext &c )
{
	#define OO_SQRT_3 0.57735025882720947f
	static const Vector bumpBasis[3] = {
		Vector( 0.81649661064147949f, 0.0f, OO_SQRT_3 ),
		Vector(  -0.40824833512306213f, 0.70710676908493042f, OO_SQRT_3 ),
		Vector(  -0.40824821591377258f, -0.7071068286895752f, OO_SQRT_3 )
	};

	float info_0[4] = { 0, 0, 0, 0 };
	Assert( m_iIndex >= 0 && m_iIndex <= 2 );
	Q_memcpy( info_0, bumpBasis[m_iIndex].Base(), sizeof(float) * 3 );

	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( info_0, 4 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}