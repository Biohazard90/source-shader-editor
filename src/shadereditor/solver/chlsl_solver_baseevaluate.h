#ifndef CHLSL_SOLVER_NORMALIZE_H
#define CHLSL_SOLVER_NORMALIZE_H

#include "editorcommon.h"

class CHLSL_Solver_BaseEvaluate : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_BaseEvaluate( HNODE nodeidx, int numInputs = 1 );
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_BaseEvaluate( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

	virtual int GetRenderOperator(){ Assert(0); return NULL; };
	virtual char *GetOperatorName(){ Assert(0); return ""; };

	int numIn;
};

#define CREATE_EVALUATE_SOLVER( className, renderOPname, codeOPname, numIn ) class className : public CHLSL_Solver_BaseEvaluate\
{\
public:\
	className( HNODE nodeidx ) : CHLSL_Solver_BaseEvaluate( nodeidx, numIn ) {};\
	CHLSL_SolverBase *Copy(){\
		return new className( *this ); };\
protected:\
	virtual int GetRenderOperator(){ return renderOPname; };\
	virtual char *GetOperatorName(){ return #codeOPname; };\
}
#define CREATE_EVALUATE_SOLVER_SIMPLE( className, renderOPname, codeOPname ) CREATE_EVALUATE_SOLVER( className, renderOPname, codeOPname, 1 )

CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Normalize, NPSOP_CALC_NORMALIZE, normalize );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Length, NPSOP_CALC_LENGTH, length );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Round, NPSOP_CALC_ROUND, round );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Fraction, NPSOP_CALC_FRAC, frac );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Floor, NPSOP_CALC_FLOOR, floor );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Ceil, NPSOP_CALC_CEIL, ceil );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Abs, NPSOP_CALC_ABS, abs );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Degrees, NPSOP_CALC_DEGREES, degrees );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Radians, NPSOP_CALC_RADIANS, radians );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Saturate, NPSOP_CALC_SATURATE, saturate );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Sign, NPSOP_CALC_SIGN, sign );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Root, NPSOP_CALC_SQRT, sqrt );

CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Log, NPSOP_CALC_LOG, log );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Log2, NPSOP_CALC_LOG2, log2 );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Log10, NPSOP_CALC_LOG10, log10 );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Exp, NPSOP_CALC_EXP, exp );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Exp2, NPSOP_CALC_EXP2, exp2 );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_ddx, NPSOP_CALC_DDX, ddx );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_ddy, NPSOP_CALC_DDY, ddy );

CREATE_EVALUATE_SOLVER( CHLSL_Solver_Power, NPSOP_CALC_POW, pow, 2 );
CREATE_EVALUATE_SOLVER( CHLSL_Solver_Step, NPSOP_CALC_STEP, step, 2 );
CREATE_EVALUATE_SOLVER( CHLSL_Solver_Min, NPSOP_CALC_MIN, min, 2 );
CREATE_EVALUATE_SOLVER( CHLSL_Solver_Max, NPSOP_CALC_MAX, max, 2 );
CREATE_EVALUATE_SOLVER( CHLSL_Solver_FMod, NPSOP_CALC_FMOD, fmod, 2 );

CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Sin, NPSOP_CALC_SIN, sin );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_ASin, NPSOP_CALC_ASIN, asin );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_SinH, NPSOP_CALC_SINH, sinh );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Cos, NPSOP_CALC_COS, cos );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_ACos, NPSOP_CALC_ACOS, acos );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_CosH, NPSOP_CALC_COSH, cosh );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_Tan, NPSOP_CALC_TAN, tan );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_ATan, NPSOP_CALC_ATAN, atan );
CREATE_EVALUATE_SOLVER_SIMPLE( CHLSL_Solver_TanH, NPSOP_CALC_TANH, tanh );
CREATE_EVALUATE_SOLVER( CHLSL_Solver_ATan2, NPSOP_CALC_ATAN2, atan2, 2 );

CREATE_EVALUATE_SOLVER( CHLSL_Solver_Distance, NPSOP_CALC_DIST, distance, 2 );

#endif