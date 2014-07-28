#ifndef CHLSL_SOLVER_MATRICES_H
#define CHLSL_SOLVER_MATRICES_H

#include "editorcommon.h"

class CHLSL_Solver_MVP : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_MVP( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_MVP( *this ); };

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
};


class CHLSL_Solver_VP : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_VP( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_VP( *this ); };

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
};


class CHLSL_Solver_M : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_M( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_M( *this ); };

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
};



class CHLSL_Solver_VM : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_VM( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_VM( *this ); };

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
};




class CHLSL_Solver_FVP : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_FVP( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_FVP( *this ); };

	//virtual bool IsRenderable(){ return false; };

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );
};



class CHLSL_Solver_CMatrix : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_CMatrix( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_CMatrix( *this ); };

	//virtual bool IsRenderable(){ return false; };

	void SetMatrixID( int i ){ m_iMatrixID = i; };

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	int m_iMatrixID;
};

#endif