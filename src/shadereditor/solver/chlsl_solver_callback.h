#ifndef CHLSL_SOLVER_CALLBACK_H
#define CHLSL_SOLVER_CALLBACK_H

#include "editorcommon.h"

class CHLSL_Solver_Callback : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Callback( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Callback( *this ); };
	CHLSL_Solver_Callback( const CHLSL_Solver_Callback &o );
	~CHLSL_Solver_Callback()
	{
		delete [] szName;
	};

	void SetState( int index, const char *name, int numComps );

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	char *szName;
	int iCallbackIndex;
	int iNumComps;
};

#endif