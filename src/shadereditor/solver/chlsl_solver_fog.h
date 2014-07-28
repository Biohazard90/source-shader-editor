#ifndef CHLSL_SOLVER_FOG_H
#define CHLSL_SOLVER_FOG_H

#include "editorcommon.h"

class CHLSL_Solver_Fog : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Fog( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Fog( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );
};

#endif