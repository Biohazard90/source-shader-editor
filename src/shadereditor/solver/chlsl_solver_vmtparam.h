#ifndef CHLSL_SOLVER_VMTPARAM_H
#define CHLSL_SOLVER_VMTPARAM_H

#include "editorcommon.h"

class CHLSL_Solver_VParam_Mutable : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_VParam_Mutable( HNODE nodeidx );
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_VParam_Mutable( *this ); };

	void SetState( int pIdx, int comps );
	void SetComponentInfo( const TokenChannels_t &info );

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	int m_iParamIndex;
	int m_iNumComponents;
	TokenChannels_t m_ConstantData;
};



class CHLSL_Solver_VParam_Static : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_VParam_Static( HNODE nodeidx );
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_VParam_Static( *this ); };
	CHLSL_Solver_VParam_Static( const CHLSL_Solver_VParam_Static &o );
	~CHLSL_Solver_VParam_Static();

	void SetState( char *name, int comps );
	void SetComponentInfo( const TokenChannels_t &info );

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );

	char *m_szName;
	int m_iNumComponents;
	TokenChannels_t m_ConstantData;
};

#endif