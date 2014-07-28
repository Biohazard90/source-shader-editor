#ifndef CHLSL_SOLVER_M_COMP_H
#define CHLSL_SOLVER_M_COMP_H

#include "editorcommon.h"

class CHLSL_Solver_MComp : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_MComp( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_MComp( *this ); };

	void SetState( int mtype, bool doC );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

	int iTargetMatrix;
	bool bColumns;
};

#endif