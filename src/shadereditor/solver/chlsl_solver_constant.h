#ifndef CHLSL_SOLVER_CONSTANT_H
#define CHLSL_SOLVER_CONSTANT_H

#include "editorcommon.h"

class CHLSL_Solver_Constant : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Constant( HNODE nodeidx );
	CHLSL_SolverBase *Copy();
	CHLSL_Solver_Constant( const CHLSL_Solver_Constant& o );

	void SetComponentInfo( const TokenChannels_t &info );

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	//virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

	TokenChannels_t m_ConstantData;
};


class CHLSL_Solver_Random : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Random( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Random( *this ); };

	void SetState( int numComp, float _min, float _max )
	{
		m_iNumComp = numComp;
		m_flValue_Min = _min;
		m_flValue_Max = _max;
	};

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	void GetHash( char *out, int maxlen );

	int m_iNumComp;
	float m_flValue_Min;
	float m_flValue_Max;
};





class CHLSL_Solver_BumpBasis : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_BumpBasis( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_BumpBasis( *this ); };

	void SetState( int idx )
	{
		m_iIndex = idx;
	};

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

	int m_iIndex;
};

#endif