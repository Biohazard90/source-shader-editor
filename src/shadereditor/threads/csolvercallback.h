#ifndef CSOLVERCALLBACK_H
#define CSOLVERCALLBACK_H

#include "editorCommon.h"

class CBaseThread;


class CAutoSolverCallback : public ISolverRequester
{
public:
	CAutoSolverCallback();
	~CAutoSolverCallback();

	HAUTOSOLVER GetRequesterIDX();
	static CAutoSolverCallback *GetSolverClientByIdx( HAUTOSOLVER idx );

	virtual void Startup();
	virtual void Shutdown();
	virtual bool IsRunning();

	virtual void OnSolverUpdated( CNodeView *pCaller, GenericShaderData *pData,
							bool VSReady, bool PSReady );
	virtual void OnSolverErrored();

	virtual const bool IsVSReady();
	virtual const bool IsPSReady();

	virtual void SetRenderSystem( const int t );
	virtual const int GetRenderSystem();
	virtual void SetPreviewMode( const int t );
	virtual const int GetPreviewMode();

	const bool IsInSoftwareMode();

	virtual void Update3DPrevMaterial( GenericShaderData *data, const int index = 0 );
	static void CopyShaders( int flags, GenericShaderData *data );
	virtual void OnCompilationEnd( int flags, int exitcode, GenericShaderData *data );

private:
	CPlainAutoPtr< CBaseThread > m_hThread;

	bool m_bVSValid;
	bool m_bPSValid;

	int m_iRenderSystem;
	int m_iPreviewMode;

	CUtlVector< CHLSL_SolverBase* >m_hSolvers_VS;
	CUtlVector< CHLSL_SolverBase* >m_hSolvers_PS;
	CUtlVector< CHLSL_SolverBase* >m_hSolvers_UNDEF;
	GenericShaderData *pShaderData;

	void DeleteSolverStacks();

	void UploadCompilationRequest();

	HAUTOSOLVER RequesterIDX;
	static CUtlVector< CAutoSolverCallback* >m_shClients;

};

#endif