
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_Array::CHLSL_Solver_Array( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx )
{
	m_iNumComps = 0;
	m_iSize_x = m_iSize_y = 0;
	m_vecData = NULL;
}
CHLSL_Solver_Array::CHLSL_Solver_Array( const CHLSL_Solver_Array &o ) : CHLSL_SolverBase( o )
{
	m_iNumComps = o.m_iNumComps;
	m_iSize_x = o.m_iSize_x;
	m_iSize_y = o.m_iSize_y;

	const int arraySize = m_iSize_x * m_iSize_y;
	m_vecData = new Vector4D[ arraySize ];
	Q_memcpy( m_vecData, o.m_vecData, sizeof(Vector4D) * arraySize );
}
CHLSL_Solver_Array::~CHLSL_Solver_Array()
{
	delete [] m_vecData;
}

void CHLSL_Solver_Array::SetState( int iComps, int iSize_x, int iSize_y,
									Vector4D *vecData )
{
	m_iNumComps = iComps;
	m_iSize_x = iSize_x;
	m_iSize_y = iSize_y;

	const int arraySize = m_iSize_x * m_iSize_y;
	m_vecData = new Vector4D[ arraySize ];
	Q_memcpy( m_vecData, vecData, sizeof(Vector4D) * arraySize );
}

void CHLSL_Solver_Array::OnVarInit( const WriteContext_FXC &context )
{
	CHLSL_Var *target = GetTargetVar( 0 );
	target->SetName( "NOT_VALIDATED_YET", true );
}
void CHLSL_Solver_Array::OnVarInit_PostStep()
{
	CHLSL_Var *src_0 = GetSourceVar( 0 );
	CHLSL_Var *src_1 = (m_iSize_y > 1) ? GetSourceVar( 1 ) : NULL;

	CHLSL_Var *target = GetTargetVar( 0 );
	char tmp[MAXTARGC];
	Q_snprintf( tmp, sizeof( tmp ), "g_cArray_%i[%s]", GetData().iNodeIndex, src_0->GetName() );
	if ( src_1 != NULL )
	{
		char szAppend[MAX_PATH];
		Q_snprintf( szAppend, sizeof( szAppend ), "[%s]", src_1->GetName() );
		Q_strcat( tmp, szAppend, sizeof( tmp ) );
	}
	//Q_strcat( tmp, "", sizeof( tmp ) );
	target->SetName( tmp, true );
}

void CHLSL_Solver_Array::Render( Preview2DContext &c )
{
	float info_0[4] = { 1, 1, 1, 1 };
	IMaterialVar *pMatVar0 = pEditorRoot->GetUVTargetParam( NPSOP_SET_CONSTANT, 0 );
	pMatVar0->SetVecValue( info_0, 4 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_SET_CONSTANT ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}

void CHLSL_Solver_Array::OnIdentifierAlloc( IdentifierLists_t &List )
{
	SimpleArray *pArray = new SimpleArray();

	pArray->iNumComps = m_iNumComps;
	pArray->iSize_X = m_iSize_x;
	pArray->iSize_Y = m_iSize_y;
	pArray->iIndex = GetData().iNodeIndex;

	const int arraySize = m_iSize_x * m_iSize_y;
	pArray->vecData = new Vector4D[ arraySize ];
	Q_memcpy( pArray->vecData, m_vecData, sizeof(Vector4D) * arraySize );

	List.hList_Arrays.AddToTail( pArray );
}