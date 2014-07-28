#ifndef CHLSL_SOLVER_CUSTOM_H
#define CHLSL_SOLVER_CUSTOM_H

#include "editorcommon.h"

class CHLSL_Solver_Custom : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Custom( HNODE nodeidx );
	~CHLSL_Solver_Custom();
	CHLSL_Solver_Custom( const CHLSL_Solver_Custom &o );
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Custom( *this ); };

	void Init( const char *pszCode_Global, const char *pszCode_Body, int iEnvFlags,
		const char *pszFunctionName, const char *pszFilePath,
		const CUtlVector< __funcParamSetup* > &hParamsIn, const CUtlVector< __funcParamSetup* > &hParamsOut,
		CBaseNode *pCaller );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );
	virtual void Render( Preview2DContext &c );

	SimpleFunction *m_pFunc;
	int m_iEnvFlags;
};

#endif