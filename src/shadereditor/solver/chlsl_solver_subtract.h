#ifndef CHLSL_SOLVER_SUBTRACT_H
#define CHLSL_SOLVER_SUBTRACT_H

#include "editorcommon.h"

class CHLSL_Solver_Subtract : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Subtract( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Subtract( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};

class CHLSL_Solver_Invert : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Invert( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Invert( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};

#endif