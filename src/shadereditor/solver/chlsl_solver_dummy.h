#ifndef CHLSL_SOLVER_DUMMY_H
#define CHLSL_SOLVER_DUMMY_H

#include "editorcommon.h"

class CHLSL_Solver_Dummy : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Dummy( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Dummy( *this ); };

	virtual bool IsDummy(){ return true; };

	virtual bool IsRenderable(){ return false; };
protected:
};



#endif