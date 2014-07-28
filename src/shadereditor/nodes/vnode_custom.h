#ifndef CNODECUSTOM_H
#define CNODECUSTOM_H

#include "vBaseNode.h"

enum
{
	NODECUSTOM_ADDENV_LIGHTING_PS = ( 1 << 0 ),
	NODECUSTOM_ADDENV_LIGHTING_VS = ( 1 << 1 ),
};

class CNodeCustom : public CBaseNode
{
	DECLARE_CLASS( CNodeCustom, CBaseNode );

public:

	CNodeCustom( CNodeView *p );
	~CNodeCustom();

	virtual int GetNodeType(){ return HLSLNODE_UTILITY_CUSTOMCODE; };

	//virtual int UpdateInputsValid();
	//virtual void UpdateOutputs();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

	virtual void PreSolverUpdated();
	virtual int UpdateInputsValid();

	static void WriteJackDataFromKV( KeyValues *pKV, CUtlVector< __funcParamSetup* > &m_hIn, CUtlVector< __funcParamSetup* > &m_hOut,
									int maxIn, int maxOut );
	static void FillJackDataFromKV( KeyValues *pKV, CUtlVector< __funcParamSetup* > &m_hIn, CUtlVector< __funcParamSetup* > &m_hOut );
	static __funcParamSetup *AllocOutputSetup( bool bOutput, const char *name, int type = HLSLVAR_FLOAT1 );


	void UpdateFromFile();
	const char *GetFilePath();
	bool IsUsingInlineCode();

private:
	void RestoreInternal( KeyValues *pKV );

	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	void UpdateNode();

	CUtlVector< __funcParamSetup* >m_hszVarNames_In;
	CUtlVector< __funcParamSetup* >m_hszVarNames_Out;

	char *m_pCode_Global;
	char *m_pCode_Function;

	char m_szFunctionName[MAX_PATH];
	char m_szFilePath[MAX_PATH*4];
	bool m_bInline;
	int m_iEnvFlags;

	void PurgeNameVectors();

	char *GenerateDefaultName( int idx, bool bOut = false );
};


#endif