#ifndef CHLSL_SOLVER_LIGHTSCALE_H
#define CHLSL_SOLVER_LIGHTSCALE_H

#include "editorcommon.h"

class CHLSL_Solver_Lightscale : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Lightscale( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Lightscale( *this ); };

	void SetState( int lscaletype );

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	int m_iLightScale;
};

#endif