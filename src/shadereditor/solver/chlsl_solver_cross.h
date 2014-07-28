#ifndef CHLSL_SOLVER_CROSS_H
#define CHLSL_SOLVER_CROSS_H

#include "editorcommon.h"

class CHLSL_Solver_Cross : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Cross( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Cross( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};

#endif