#ifndef CHLSL_SOLVER_STD_SKINNING_H
#define CHLSL_SOLVER_STD_SKINNING_H

#include "editorcommon.h"

class CHLSL_Solver_StdSkinning : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_StdSkinning( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_StdSkinning( *this ); };

	virtual bool IsRenderable(){ return false; };

	void SetState( int m );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	int m_iState;
};


class CHLSL_Solver_StdMorph : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_StdMorph( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_StdMorph( *this ); };

	virtual bool IsRenderable(){ return false; };

	void SetState( int m );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	int m_iState;
};

#endif