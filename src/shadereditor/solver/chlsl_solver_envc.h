#ifndef CHLSL_SOLVER_TIME_H
#define CHLSL_SOLVER_TIME_H

#include "editorcommon.h"

class CHLSL_Solver_EnvC : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_EnvC( HNODE nodeidx );
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_EnvC( *this ); };

	void SetEnvironmentConstantIndex( int i, float val0 ){
		m_iEnvCID = i;
		m_flSmartVal0 = val0;
	};

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	int m_iEnvCID;
	float m_flSmartVal0;
};

#endif