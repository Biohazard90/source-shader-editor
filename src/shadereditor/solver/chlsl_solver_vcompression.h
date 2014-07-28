#ifndef CHLSL_SOLVER_VCOMPRESSION_H
#define CHLSL_SOLVER_VCOMPRESSION_H

#include "editorcommon.h"

class CHLSL_Solver_VCompression : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_VCompression( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_VCompression( *this ); };

	virtual bool IsRenderable(){ return false; };

	void SetCompressionType( int type );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	int m_iType;
};


#endif