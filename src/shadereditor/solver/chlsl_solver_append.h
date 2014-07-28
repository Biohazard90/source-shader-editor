#ifndef CHLSL_SOLVER_APPEND_H
#define CHLSL_SOLVER_APPEND_H

#include "editorcommon.h"

class CHLSL_Solver_Append : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Append( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Append( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};

#endif