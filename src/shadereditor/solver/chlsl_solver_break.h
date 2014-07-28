#ifndef CHLSL_SOLVER_BREAK_H
#define CHLSL_SOLVER_BREAK_H

#include "editorcommon.h"

class CHLSL_Solver_Break : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Break( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Break( *this ); };

	//virtual bool IsRenderable(){ return false; };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
};


class CHLSL_Solver_Clip : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Clip( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Clip( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};

#endif