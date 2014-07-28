#ifndef CHLSL_SOLVER_LERP_H
#define CHLSL_SOLVER_LERP_H

#include "editorcommon.h"

class CHLSL_Solver_Lerp : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Lerp( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Lerp( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};

#endif