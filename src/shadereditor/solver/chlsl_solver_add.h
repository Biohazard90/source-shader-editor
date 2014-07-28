#ifndef CHLSL_SOLVER_ADD_H
#define CHLSL_SOLVER_ADD_H

#include "editorcommon.h"

class CHLSL_Solver_Add : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Add( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Add( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};

#endif