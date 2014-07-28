#ifndef CHLSL_SOLVER_FLASHLIGHT_H
#define CHLSL_SOLVER_FLASHLIGHT_H

#include "editorcommon.h"

class CHLSL_Solver_Flashlight : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Flashlight( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Flashlight( *this ); };

	void SetState( bool spec );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	bool m_bSpecular;
};



class CHLSL_Solver_Flashlight_Pos : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Flashlight_Pos( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Flashlight_Pos( *this ); };

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );
};

#endif