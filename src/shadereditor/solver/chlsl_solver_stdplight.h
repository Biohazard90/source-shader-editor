#ifndef CHLSL_SOLVER_STD_PLIGHT_H
#define CHLSL_SOLVER_STD_PLIGHT_H

#include "editorcommon.h"

class CHLSL_Solver_StdPLight : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_StdPLight( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_StdPLight( *this ); };

	void SetState( bool supportsStaticL, bool doHL, bool doAO );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	bool m_bStaticLighting;
	bool m_bAOTerm;
	bool m_bHalfLambert;
};



class CHLSL_Solver_StdPSpecLight : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_StdPSpecLight( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_StdPSpecLight( *this ); };

	void SetState( bool doAO );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	bool m_bAOTerm;
};

#endif