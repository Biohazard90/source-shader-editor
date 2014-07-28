#ifndef CHLSL_SOLVER_UTILITY_H
#define CHLSL_SOLVER_UTILITY_H

#include "editorcommon.h"

class CHLSL_Solver_Utility_Declare : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Utility_Declare( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Utility_Declare( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};


class CHLSL_Solver_Utility_Assign : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Utility_Assign( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Utility_Assign( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};


#endif