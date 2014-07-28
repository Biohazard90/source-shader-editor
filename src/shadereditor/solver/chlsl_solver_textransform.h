#ifndef CHLSL_SOLVER_TEXTRANSFORM_H
#define CHLSL_SOLVER_TEXTRANSFORM_H

#include "editorcommon.h"

class CHLSL_Solver_TexTransform : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_TexTransform( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_TexTransform( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

public:
	bool bEnable_Center;
	bool bEnable_Rot;
	bool bEnable_Scale;
	bool bEnable_Trans;
};

#endif