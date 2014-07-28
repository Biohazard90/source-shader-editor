#ifndef CHLSL_SOLVER_WRITESEMANTICS_H
#define CHLSL_SOLVER_WRITESEMANTICS_H

#include "editorcommon.h"

class CHLSL_Solver_WriteSemantics : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_WriteSemantics( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_WriteSemantics( *this ); };

	virtual bool IsRenderable(){ return false; };
protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
};



#endif