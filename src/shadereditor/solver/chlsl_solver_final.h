#ifndef CHLSL_SOLVER_FINAL_H
#define CHLSL_SOLVER_FINAL_H

#include "editorcommon.h"

class CHLSL_Solver_Final : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Final( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Final( *this ); };

	virtual void SetState( int t, bool wd );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	int iTonemap;
	bool bWriteDepth;
};

#endif