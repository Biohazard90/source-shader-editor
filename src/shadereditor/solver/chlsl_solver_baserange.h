#ifndef CHLSL_SOLVER_RANGE_H
#define CHLSL_SOLVER_RANGE_H

#include "editorcommon.h"

class CHLSL_Solver_BaseRange : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_BaseRange( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_BaseRange( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

	virtual int GetRenderOperator(){ Assert(0); return NULL; };
};



class CHLSL_Solver_Smoothstep : public CHLSL_Solver_BaseRange
{
public:
	CHLSL_Solver_Smoothstep( HNODE nodeidx ) : CHLSL_Solver_BaseRange( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Smoothstep( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual int GetRenderOperator(){ return NPSOP_CALC_SMOOTHSTEP; };

};


class CHLSL_Solver_Clamp : public CHLSL_Solver_BaseRange
{
public:
	CHLSL_Solver_Clamp( HNODE nodeidx ) : CHLSL_Solver_BaseRange( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Clamp( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual int GetRenderOperator(){ return NPSOP_CALC_CLAMP; };

};

#endif