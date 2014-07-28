#ifndef CHLSL_SOLVER_VECREF_H
#define CHLSL_SOLVER_VECREF_H

#include "editorcommon.h"

class CHLSL_Solver_VectorReflect : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_VectorReflect( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_VectorReflect( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};



class CHLSL_Solver_VectorRefract : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_VectorRefract( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_VectorRefract( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};

#endif