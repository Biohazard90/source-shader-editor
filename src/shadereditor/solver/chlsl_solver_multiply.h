#ifndef CHLSL_SOLVER_MULTIPLY_H
#define CHLSL_SOLVER_MULTIPLY_H

#include "editorcommon.h"

class CHLSL_Solver_Multiply : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Multiply( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Multiply( *this ); };

	void SetMatrixRotationOnly( bool b ){
		m_bMatrixRotationOnly = b; };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

	bool m_bMatrixRotationOnly;
};

#endif