#ifndef CCOMPILE_THREAD_H
#define CCOMPILE_THREAD_H

#include "cbase.h"
#include "editorCommon.h"

enum
{
	ACTIVEFLAG_VS = ( 1 << 0 ),
	ACTIVEFLAG_VS_POS = ( 1 << 1 ),
	ACTIVEFLAG_PS = ( 1 << 2 ),
};

class CCompileThread : public CBaseThread
{
public:
	CCompileThread();
	~CCompileThread();
	static CCompileThread *CreateWriteFXCThread();

	int Run();

	CMessageQueue< __threadcmds_CompileCommand* > m_Queue_CompileCommand;

private:

	CUtlVector< __threadcmds_CompileCommand* > m_hCurrentMessages;
	void DestroyCompilerMessage( __threadcmds_CompileCommand *msg );
	void UniquifyCommandList();

	CUtlVector< CHLSL_SolverBase* > *_solvers_vs;
	CUtlVector< CHLSL_SolverBase* > *_solvers_ps;
	GenericShaderData *_shaderData;
	HAUTOSOLVER *_target;

	CUtlVector< SimpleCombo* >_combos_vs;
	CUtlVector< SimpleCombo* >_combos_ps;

	void WriteFXCFile_VS(bool bPosOverride, CUtlBufferEditor *codeBuff = NULL );
	void WriteFXCFile_PS( CUtlBufferEditor *codeBuff = NULL );
	void WriteFileLists(int activefiles, CUtlBufferEditor *codeBuff = NULL );

	void WriteIncludes( bool bPS, CUtlBufferEditor &buf, CUtlVector< CHLSL_SolverBase* > &m_hSolvers );
	void WriteCommon( bool bPS, CUtlBufferEditor &buf, CUtlVector< CHLSL_SolverBase* > &m_hSolvers, bool bPosOverride, CUtlBufferEditor *codeBuff = NULL );

	bool StartCompiler();

	int curFlags;
};

#endif