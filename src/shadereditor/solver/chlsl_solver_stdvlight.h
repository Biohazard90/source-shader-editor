#ifndef CHLSL_SOLVER_STD_VLIGHT_H
#define CHLSL_SOLVER_STD_VLIGHT_H

#include "editorcommon.h"

class CHLSL_Solver_StdVLight : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_StdVLight( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_StdVLight( *this ); };

	virtual bool IsRenderable(){ return false; };

	void SetState( bool supportsStaticL, bool doHL );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	bool m_bStaticLighting;
	bool m_bHalfLambert;
};

#endif