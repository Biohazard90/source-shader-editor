#ifndef CHLSL_SOLVER_DIVIDE_H
#define CHLSL_SOLVER_DIVIDE_H

#include "editorcommon.h"

class CHLSL_Solver_Divide : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Divide( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Divide( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};

#endif