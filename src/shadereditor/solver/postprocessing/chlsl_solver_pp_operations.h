#ifndef CHLSL_SOLVER_PP_OPERATIONS_H
#define CHLSL_SOLVER_PP_OPERATIONS_H

#include "editorcommon.h"

class CHLSL_Solver_PP_ClearBuffers : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_ClearBuffers( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx ){};
	virtual CHLSL_Solver_PP_ClearBuffers *Copy(){
		return new CHLSL_Solver_PP_ClearBuffers( *this ); };

	void Init( const TokenChannels_t &values, bool bColor, bool bDepth );

protected:
	virtual void OnExecuteCode( const RunCodeContext &context );

	TokenChannels_t m_iChannelInfo;
	bool bClearColor;
	bool bClearDepth;
};


class CHLSL_Solver_PP_VP_Push : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_VP_Push( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx ){};
	virtual CHLSL_Solver_PP_VP_Push *Copy(){
		return new CHLSL_Solver_PP_VP_Push( *this ); };

protected:
	virtual void OnExecuteCode( const RunCodeContext &context );
};


class CHLSL_Solver_PP_VP_Pop : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_VP_Pop( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx ){};
	virtual CHLSL_Solver_PP_VP_Pop *Copy(){
		return new CHLSL_Solver_PP_VP_Pop( *this ); };

protected:
	virtual void OnExecuteCode( const RunCodeContext &context );
};

class CHLSL_Solver_PP_VP_SetRT : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_VP_SetRT( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx ){};
	virtual CHLSL_Solver_PP_VP_SetRT *Copy(){
		return new CHLSL_Solver_PP_VP_SetRT( *this ); };

protected:
	virtual void OnExecuteCode( const RunCodeContext &context );
};

class CHLSL_Solver_PP_CopyRT : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_CopyRT( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx ){};
	virtual CHLSL_Solver_PP_CopyRT *Copy(){
		return new CHLSL_Solver_PP_CopyRT( *this ); };

protected:
	virtual void OnExecuteCode( const RunCodeContext &context );
};

class CHLSL_Solver_PP_UpdateFB : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_UpdateFB( HNODE nodeidx ) : CHLSL_Solver_PP_Base( nodeidx ){};
	virtual CHLSL_Solver_PP_UpdateFB *Copy(){
		return new CHLSL_Solver_PP_UpdateFB( *this ); };

protected:
	virtual void OnExecuteCode( const RunCodeContext &context );
};

#endif