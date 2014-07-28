
#include "cbase.h"
#include "editorcommon.h"

CNodeArray::CNodeArray( CNodeView *p ) : BaseClass( "Array", p )
{
	m_iSize_X = 6;
	m_iSize_Y = 1;

	m_flMinSizeX = NODEDEFSIZE_SMALL;
	m_flMinSizeY = NODEDEFSIZE_SMALL;
	m_flMinSizePREVIEW_X = NODEDEFSIZE_SMALL;

	m_iDataTypeFlag = HLSLVAR_FLOAT1;
	m_vecValues = new Vector4D[ m_iSize_X * m_iSize_Y ];
	Q_memset( m_vecValues, 0, sizeof( Vector4D ) * ( m_iSize_X * m_iSize_Y ) );

	GenerateJacks_Output( 1 );

	UpdateNode();
}

//CNodeArray::CNodeArray( const CNodeArray &other ) : BaseClass( other )
//{
//	m_iSize_X = other.m_iSize_X;
//	m_iSize_Y = other.m_iSize_Y;
//
//	m_iDataTypeFlag = other.m_iDataTypeFlag;
//	m_vecValues = NULL;
//
//	if ( other.m_vecValues )
//	{
//		int vecSize = m_iSize_X * m_iSize_Y;
//		m_vecValues = new Vector4D[ vecSize ];
//		Q_memcpy( m_vecValues, other.m_vecValues, sizeof( Vector4D ) * vecSize );
//	}
//}

CNodeArray::~CNodeArray()
{
	delete [] m_vecValues;
}

void CNodeArray::UpdateNode()
{
	const bool bIs2D = m_iSize_Y > 1;

	CUtlVector< BridgeRestoreInfo* >m_hRestoreBridges;
	CreateBridgeRestoreData_In( CBaseNode::BY_INDEX, m_hRestoreBridges );

	GenerateJacks_Input( bIs2D ? 2 : 1 );

	if ( bIs2D )
	{
		LockJackInput_Flags( 0, HLSLVAR_FLOAT1, "X" );
		LockJackInput_Flags( 1, HLSLVAR_FLOAT1, "Y" );
	}
	else
		LockJackInput_Flags( 0, HLSLVAR_FLOAT1, "X" );

	RestoreBridgesFromList_In( m_hRestoreBridges );

	LockJackOutput_Flags( 0, m_iDataTypeFlag, "Var" );
}

KeyValues *CNodeArray::AllocateKeyValues( int NodeIndex )
{
	KeyValues *pKV = BaseClass::AllocateKeyValues( NodeIndex );

	pKV->SetInt( "i_array_size_x", m_iSize_X );
	pKV->SetInt( "i_array_size_y", m_iSize_Y );
	pKV->SetInt( "i_array_datatype", m_iDataTypeFlag );

	const int arraySize = m_iSize_X * m_iSize_Y;
	char tmp[ MAX_PATH ];
	for ( int i = 0; i < arraySize; i++ )
	{
		const Vector4D &vec = m_vecValues[ i ];
		Q_snprintf( tmp, sizeof( tmp ), "%f %f %f %f", vec[0], vec[1], vec[2], vec[3] );
		pKV->SetString( VarArgs( "pfl_arraydata_%03i", i ), tmp );
	}

	return pKV;
}
void CNodeArray::RestoreFromKeyValues_Specific( KeyValues *pKV )
{
	m_iSize_X = pKV->GetInt( "i_array_size_x", m_iSize_X );
	m_iSize_Y = pKV->GetInt( "i_array_size_y", m_iSize_Y );
	m_iDataTypeFlag = pKV->GetInt( "i_array_datatype", m_iDataTypeFlag );

	delete [] m_vecValues;
	const int arraySize = m_iSize_X * m_iSize_Y;
	m_vecValues = new Vector4D[ arraySize ];
	::SimpleTokenize tokens;
	for ( int i = 0; i < arraySize; i++ )
	{
		const char *szData = pKV->GetString( VarArgs( "pfl_arraydata_%03i", i ) );
		tokens.Tokenize( szData );
		m_vecValues[i].Init();
		for ( int t = 0; t < tokens.Count() && t < 4; t++ )
		{
			float flVal = atof( tokens.GetToken( t ) );
			m_vecValues[i][t] = flVal;
		}
	}

	UpdateNode();
	OnUpdateHierachy( NULL, NULL );
}

bool CNodeArray::CreateSolvers(GenericShaderData *ShaderData)
{
	if ( GetNumJacks_In_Connected() < GetNumJacks_In() )
		return false;

	if ( m_iSize_X < 1 || m_iSize_Y < 1 || m_vecValues == NULL )
		return false;

	CJack *pJ_Out = GetJack_Out( 0 );
	const int res = pJ_Out->GetResourceType();
	CHLSL_Var *tg = pJ_Out->AllocateVarFromSmartType();
	tg->MakeConstantOnly();

	CHLSL_Solver_Array *solver = new CHLSL_Solver_Array( GetUniqueIndex() );
	solver->SetResourceType( res );
	solver->SetState( ::GetVarFlagsVarValue( m_iDataTypeFlag ), m_iSize_X, m_iSize_Y, m_vecValues );
	solver->AddTargetVar( tg );
	for ( int i = 0; i < GetNumJacks_In(); i++ )
		solver->AddSourceVar( GetJack_In( i )->GetTemporaryVarTarget_End() );
	AddSolver( solver );
	return true;
}