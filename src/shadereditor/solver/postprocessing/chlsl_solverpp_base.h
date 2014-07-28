#ifndef CHLSL_SOLVER_PP_BASE_H
#define CHLSL_SOLVER_PP_BASE_H

#include "editorcommon.h"

class CHLSL_Solver_PP_Base : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_PP_Base( HNODE nodeidx );
	virtual CHLSL_Solver_PP_Base *Copy(){
		return new CHLSL_Solver_PP_Base( *this ); };
	//CHLSL_Solver_PP_Base( const CHLSL_Solver_PP_Base& o );

	virtual bool IsRenderable(){ return false; };
	virtual bool IsSceneSolver( const RunCodeContext &context ){ return false; };

	virtual void Invoke_ExecuteCode( const RunCodeContext &context );

protected:

	virtual void OnExecuteCode( const RunCodeContext &context ){};

	void BlitRTRect( const RunCodeContext &context );
};

class CHLSL_Solver_PP_Start : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_Start( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx ) {};
	virtual CHLSL_Solver_PP_Start *Copy(){
		return new CHLSL_Solver_PP_Start( *this ); };
	//CHLSL_Solver_PP_Base( const CHLSL_Solver_PP_Base& o );

protected:
	//virtual void OnVarInit( const WriteContext_FXC &context );
	//virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	//virtual void Render( Preview2DContext &c );
	virtual void OnExecuteCode( const RunCodeContext &context );
};

#endif