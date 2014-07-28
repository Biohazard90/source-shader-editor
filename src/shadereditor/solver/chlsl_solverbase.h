#ifndef CHLSL_SOLVER_BASE_H
#define CHLSL_SOLVER_BASE_H

#include "editorcommon.h"

struct IdentifierLists_t;

struct SolverInfo_t
{
	SolverInfo_t();
	~SolverInfo_t();
	SolverInfo_t( const SolverInfo_t &o );

	HNODE iNodeIndex;

	int _IntValue_0;
	int _IntValue_1;

	bool m_bValid;
};

class CHLSL_SolverBase
{
public:
	CHLSL_SolverBase( HNODE nodeidx );
	virtual ~CHLSL_SolverBase();

	CHLSL_SolverBase( const CHLSL_SolverBase& o );

	virtual CHLSL_SolverBase *Copy();

	virtual bool IsDummy(){ return false; };

	const ResourceType_t &GetResourceType();
	void SetResourceType( const int &type );

	virtual void ReconnectVariables(	CHLSL_SolverBase *shadow,
										CUtlVector< CHLSL_Var* > &m_hListOld,
										CUtlVector< CHLSL_Var* > &m_hListNew );

	virtual void AddSourceVar( CHLSL_Var *var );
	virtual void AddTargetVar( CHLSL_Var *var );

	int GetNumSourceVars();
	int GetNumTargetVars();

	CHLSL_Var *GetSourceVar( int idx );
	CHLSL_Var *GetTargetVar( int idx );

	SolverInfo_t &GetData();

	virtual void ClearInfo();

	virtual void Invoke_VarInit( const WriteContext_FXC &context );
	virtual void Invoke_VarInit_PostStep();
	virtual void Invoke_WriteFXC( bool bIsPixelShader, WriteContext_FXC &context );

	virtual void Invoke_RenderInit();
	virtual void Invoke_Render( Preview2DContext &c );
	virtual void Render( Preview2DContext &c );
	virtual bool IsRenderable(){ return true; };

	virtual void Invoke_IdentifierAllocate( IdentifierLists_t &List );

	virtual void Invoke_ExecuteCode( const RunCodeContext &context );

	const int &GetMapIndex(){return m_iMapIndex;};

protected:
	virtual void OnRenderInit();
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void OnVarInit_PostStep();
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );
	virtual void OnExecuteCode( const RunCodeContext &context );

	//SolverResources *m_pCurrentSolverResource;

	//SolverInfo_t *pInfo;
	SolverInfo_t pInfo;
	CUtlVector< CHLSL_Var* >m_hVar_SourceVars;
	CUtlVector< CHLSL_Var* >m_hVar_TargetVars;

	ResourceType_t m_iResourceType;

	void UpdateTargetVarToReflectMapIndex( int var );
	void SetUVParamBySourceVar( int operatoridx, int uvparam, int sourcevar );

	int m_iMapIndex;
};



#endif