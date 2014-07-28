
#include "cbase.h"
#include "editorCommon.h"

CHLSL_Solver_Swizzle::CHLSL_Solver_Swizzle( HNODE nodeidx  ) : CHLSL_SolverBase( nodeidx )
{
	//for ( int i = 0; i < 4; i++ )
	//	iCompOrder[ i ] = i;
	//targetSize = 4;
	m_SwizzleData.SetDefaultChannels();
}
CHLSL_Solver_Swizzle::CHLSL_Solver_Swizzle( const CHLSL_Solver_Swizzle& o ) : CHLSL_SolverBase( o )
{
	//for ( int i = 0; i < 4; i++ )
	//	iCompOrder[ i ] = o.iCompOrder[ i ];
	//targetSize = o.targetSize;
	m_SwizzleData = o.m_SwizzleData;
}
CHLSL_SolverBase *CHLSL_Solver_Swizzle::Copy()
{
	return new CHLSL_Solver_Swizzle( *this );
}

void CHLSL_Solver_Swizzle::SetComponentInfo( const TokenChannels_t &info )
{
	m_SwizzleData = info;
	//Assert( slot < 4 );
	//iCompOrder[ slot ] = targetComponent;
}
//void CHLSL_Solver_Swizzle::SetTargetSize( int s )
//{
//	targetSize = s;
//}

void CHLSL_Solver_Swizzle::OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context )
{
	CHLSL_Var *tg = GetTargetVar( 0 );
	tg->DeclareMe(context);
	CHLSL_Var *src1 = GetSourceVar( 0 );

	char tmp[MAXTARGC];
	Q_snprintf( tmp, MAXTARGC, "%s = ", tg->GetName() );
	context.buf_code.PutString( tmp );

	if ( !m_SwizzleData.bHasAnyValues )
	{
		Q_snprintf( tmp, MAXTARGC, "%s", src1->GetName() );
		context.buf_code.PutString( tmp );
		for ( int i = 0; i < m_SwizzleData.iActiveChannels; i++ )
		{
			if ( i == 0 )
				context.buf_code.PutString( "." );

			Q_snprintf( tmp, MAXTARGC, "%c", GetCharFromChannelNum( m_SwizzleData.iChannel[i] ) );
			context.buf_code.PutString(tmp);
		}
	}
	else
	{
		context.buf_code.PutString( ::GetVarTypeNameCode( m_SwizzleData.iActiveChannels-1 ) );
		context.buf_code.PutString( "( " );
		for ( int i = 0; i < m_SwizzleData.iActiveChannels; i++ )
		{
			if ( m_SwizzleData.bChannelAsValue[ i ] )
			{
				Q_snprintf( tmp, MAXTARGC, "%f", m_SwizzleData.flValue[ i ] );
				context.buf_code.PutString(tmp);
			}
			else
			{
				Q_snprintf( tmp, MAXTARGC, "%s.%c", src1->GetName(), GetCharFromChannelNum( m_SwizzleData.iChannel[i] ) );
				context.buf_code.PutString(tmp);
			}
			if ( i < (m_SwizzleData.iActiveChannels-1) )
				context.buf_code.PutString(", ");
		}
		context.buf_code.PutString( " )" );
	}

	context.buf_code.PutString(";\n");
}
void CHLSL_Solver_Swizzle::Render( Preview2DContext &c )
{
	SetUVParamBySourceVar( NPSOP_CALC_SWIZZLE, 0, 0 );

	float info_5[4] = { 0, 0, 0, 0 };
	for ( int i = 0; i < 4; i++ )
	{
		IMaterialVar *pMatVar = pEditorRoot->GetUVTargetParam( NPSOP_CALC_SWIZZLE, 1 + i );
		if ( !m_SwizzleData.IsActive() )
			pMatVar->SetVecValue( i == 0, i == 1, i == 2, i == 3 );
		else if ( i < m_SwizzleData.iActiveChannels && !m_SwizzleData.bChannelAsValue[i] )
		{
			int num = m_SwizzleData.iChannel[ i ];
			pMatVar->SetVecValue( num == 0, num == 1, num == 2, num == 3 );
		}
		else if ( m_SwizzleData.bChannelAsValue[ i ] )
		{
			info_5[ i ] = m_SwizzleData.flValue[ i ];
			pMatVar->SetVecValue( 0, 0, 0, 0 );
		}
		else
			pMatVar->SetVecValue( 0, 0, 0, 0 );
	}

	IMaterialVar *pMatVar5 = pEditorRoot->GetUVTargetParam( NPSOP_CALC_SWIZZLE, 5 );
	pMatVar5->SetVecValue( info_5, 4 );

	CNodeView::RenderSingleSolver( c, pEditorRoot->GetOperatorMaterial( NPSOP_CALC_SWIZZLE ) );
	UpdateTargetVarToReflectMapIndex( 0 );
}
