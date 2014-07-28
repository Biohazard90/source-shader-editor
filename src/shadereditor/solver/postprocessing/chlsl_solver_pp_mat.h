#ifndef CHLSL_SOLVER_PP_MAT_H
#define CHLSL_SOLVER_PP_MAT_H

#include "editorcommon.h"

class CHLSL_Solver_PP_Mat : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_Mat( HNODE nodeidx );
	~CHLSL_Solver_PP_Mat();
	CHLSL_Solver_PP_Mat( const CHLSL_Solver_PP_Mat &o );
	virtual CHLSL_Solver_PP_Mat *Copy(){
		return new CHLSL_Solver_PP_Mat( *this ); };

	void Init( bool bInline, const char *pszPath, const char *pszScript, const char *pszUserName );
	virtual bool IsSceneSolver( const RunCodeContext &context ){ return context.IsSceneMode(); };

	const bool IsInline();
	const char *GetNodeName();
	const char *GetMaterialName();
	IMaterial *GetMaterial();
	KeyValues *AccessKeyValues();

protected:
	virtual void OnExecuteCode( const RunCodeContext &context );

	bool m_bInline;
	char *m_pszInfoString;
	char *m_pszNodeName;

private:

	void InitMaterialInfo();
	void DestroyMaterialInfo();

	IMaterial *pMat;
	KeyValues *pKVMat;
	char *m_pMatName;
};

#endif