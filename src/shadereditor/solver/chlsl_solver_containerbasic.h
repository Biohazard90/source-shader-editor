#ifndef CHLSL_SOLVER_CONTAINERBASIC_H
#define CHLSL_SOLVER_CONTAINERBASIC_H

#include "editorcommon.h"

class CHLSL_Solver_ContainerBasic : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_ContainerBasic( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_ContainerBasic( *this ); };

	virtual bool IsRenderable(){ return false; };
protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
};



class CHLSL_Solver_LoopIntro : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_LoopIntro( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_LoopIntro( *this ); };

	int m_iCondition;
	int m_iValue_Start;
	int m_iValue_End;
	bool bDecrement;

	virtual bool IsRenderable(){ return false; };
protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
};



class CHLSL_Solver_Iterator : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Iterator( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Iterator( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};


class CHLSL_Solver_LoopOutro : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_LoopOutro( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_LoopOutro( *this ); };

	virtual bool IsRenderable(){ return false; };
protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
};


class CHLSL_Solver_ComboIntro : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_ComboIntro( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) { m_szComboName = NULL; };
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_ComboIntro( *this ); };
	CHLSL_Solver_ComboIntro( const CHLSL_Solver_ComboIntro &o );
	~CHLSL_Solver_ComboIntro()
	{
		delete [] m_szComboName;
	};

	void SetState( int cond, const char *name, int val, bool bstatic );

	virtual bool IsRenderable(){ return false; };
protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	int m_iCondition;
	bool m_bStatic;
	char *m_szComboName;
	int m_iValue;
};
class CHLSL_Solver_ComboOutro : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_ComboOutro( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_ComboOutro( *this ); };

	virtual bool IsRenderable(){ return false; };
protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
};




class CHLSL_Solver_ConditionIntro : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_ConditionIntro( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_ConditionIntro( *this ); };

	int m_iCondition;

	virtual bool IsRenderable(){ return false; };
protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
};

#endif