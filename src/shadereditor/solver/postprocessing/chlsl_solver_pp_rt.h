#ifndef CHLSL_SOLVER_PP_RT_H
#define CHLSL_SOLVER_PP_RT_H

#include "editorcommon.h"

class CHLSL_Solver_PP_RT : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_RT( HNODE nodeidx );
	virtual CHLSL_Solver_PP_RT *Copy(){
		return new CHLSL_Solver_PP_RT( *this ); };

	void Init( const char *rtname );
	virtual bool IsSceneSolver( const RunCodeContext &context ){ return context.IsSceneMode(); };

protected:
	virtual void OnExecuteCode( const RunCodeContext &context );

	char m_szRTName[RT_NAME_MAX];
};

#endif