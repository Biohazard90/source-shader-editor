#ifndef CNODE_NORMALIZE_H
#define CNODE_NORMALIZE_H

#include "vBaseNode.h"

class CNodeBaseEvaluate : public CBaseNode
{
	DECLARE_CLASS( CNodeBaseEvaluate, CBaseNode );

public:

	CNodeBaseEvaluate( const char *name, CNodeView *p, int LockOutput, int numInputs = 1, bool bSmall = false, int LockInput = -1 );
	~CNodeBaseEvaluate();

	virtual int GetNodeType(){ Assert(0); return HLSLNODE_MATH_NORMALIZE; };

	virtual int UpdateInputsValid();
	virtual void UpdateOutputs();

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);
protected:
	virtual CHLSL_SolverBase *AllocRangeSolver(){ Assert(0); return NULL; };

};

#define CREATE_EVALUATE_NODE( className, nodeName, nodeType, solverclassName, lockType, numIn, isSmall, lockTypeIn ) class className : public CNodeBaseEvaluate\
{\
	DECLARE_CLASS( className, CNodeBaseEvaluate );\
public:\
	className( CNodeView *p ) : BaseClass( #nodeName, p, lockType, numIn, isSmall, lockTypeIn ){};\
	virtual int GetNodeType(){ return nodeType; };\
protected:\
	virtual CHLSL_SolverBase *AllocRangeSolver(){ return new solverclassName( GetUniqueIndex() ); };\
}
#define CREATE_EVALUATE_NODE_FAST( name, nodeType, lockType ) CREATE_EVALUATE_NODE( CNode##name, name, nodeType, CHLSL_Solver_##name, lockType, 1, false, -1 )
#define CREATE_EVALUATE_NODE_FAST_SETIN( name, nodeType, lockType, numIn ) CREATE_EVALUATE_NODE( CNode##name, name, nodeType, CHLSL_Solver_##name, lockType, numIn, false, -1 )

#define CREATE_EVALUATE_NODE_FAST_SMALL( name, nodeType, lockType ) CREATE_EVALUATE_NODE( CNode##name, name, nodeType, CHLSL_Solver_##name, lockType, 1, true, -1 )
#define CREATE_EVALUATE_NODE_FAST_SETIN_SMALL( name, nodeType, lockType, numIn ) CREATE_EVALUATE_NODE( CNode##name, name, nodeType, CHLSL_Solver_##name, lockType, numIn, true, -1 )

#define CREATE_EVALUATE_NODE_FAST_SETIN_SMALL_LOCKIN( name, nodeType, lockType, numIn, lockTypeIn ) CREATE_EVALUATE_NODE( CNode##name, name, nodeType, CHLSL_Solver_##name, lockType, numIn, true, lockTypeIn )

//CREATE_EVALUATE_NODE( CNodeNormalize, Normalize, HLSLNODE_MATH_NORMALIZE, CHLSL_Solver_Normalize, 0 );

CREATE_EVALUATE_NODE_FAST( Normalize, HLSLNODE_MATH_NORMALIZE, 0 );
CREATE_EVALUATE_NODE_FAST( Length, HLSLNODE_MATH_LENGTH, HLSLVAR_FLOAT1 );
CREATE_EVALUATE_NODE_FAST( Round, HLSLNODE_MATH_ROUND, 0 );
CREATE_EVALUATE_NODE_FAST( Fraction, HLSLNODE_MATH_FRAC, 0 );
CREATE_EVALUATE_NODE_FAST( Floor, HLSLNODE_MATH_FLOOR, 0 );
CREATE_EVALUATE_NODE_FAST( Ceil, HLSLNODE_MATH_CEIL, 0 );
CREATE_EVALUATE_NODE_FAST( Abs, HLSLNODE_MATH_ABS, 0 );
CREATE_EVALUATE_NODE_FAST( Saturate, HLSLNODE_MATH_SATURATE, 0 );
CREATE_EVALUATE_NODE_FAST( Root, HLSLNODE_MATH_SQRT, 0 );

CREATE_EVALUATE_NODE_FAST_SMALL( Degrees, HLSLNODE_MATH_DEGREES, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( Radians, HLSLNODE_MATH_RADIANS, 0 );
CREATE_EVALUATE_NODE_FAST( Sign, HLSLNODE_MATH_SIGN, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( Log, HLSLNODE_MATH_LOG, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( Log2, HLSLNODE_MATH_LOG2, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( Log10, HLSLNODE_MATH_LOG10, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( Exp, HLSLNODE_MATH_EXP, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( Exp2, HLSLNODE_MATH_EXP2, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( ddx, HLSLNODE_MATH_DDX, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( ddy, HLSLNODE_MATH_DDY, 0 );

CREATE_EVALUATE_NODE_FAST_SETIN( Power, HLSLNODE_MATH_POW, 0, 2 );
CREATE_EVALUATE_NODE_FAST_SETIN( Step, HLSLNODE_MATH_STEP, 0, 2 );
CREATE_EVALUATE_NODE_FAST_SETIN( Min, HLSLNODE_MATH_MIN, 0, 2 );
CREATE_EVALUATE_NODE_FAST_SETIN( Max, HLSLNODE_MATH_MAX, 0, 2 );
CREATE_EVALUATE_NODE_FAST_SETIN( FMod, HLSLNODE_MATH_FMOD, 0, 2 );

CREATE_EVALUATE_NODE_FAST_SMALL( Sin, HLSLNODE_MATH_SIN, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( ASin, HLSLNODE_MATH_ASIN, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( SinH, HLSLNODE_MATH_SINH, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( Cos, HLSLNODE_MATH_COS, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( ACos, HLSLNODE_MATH_ACOS, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( CosH, HLSLNODE_MATH_COSH, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( Tan, HLSLNODE_MATH_TAN, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( ATan, HLSLNODE_MATH_ATAN, 0 );
CREATE_EVALUATE_NODE_FAST_SMALL( TanH, HLSLNODE_MATH_TANH, 0 );
CREATE_EVALUATE_NODE_FAST_SETIN_SMALL( ATan2, HLSLNODE_MATH_ATAN2, 0, 2 );

CREATE_EVALUATE_NODE_FAST_SETIN_SMALL_LOCKIN( Distance, HLSLNODE_VECTOR_DISTANCE, HLSLVAR_FLOAT1, 2, HLSLJACKFLAGS_F1_TO_F4 );

#endif