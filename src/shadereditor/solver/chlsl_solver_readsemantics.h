#ifndef CHLSL_SOLVER_READSEMANTICS_H
#define CHLSL_SOLVER_READSEMANTICS_H

#include "editorcommon.h"

class CHLSL_Solver_ReadSemantics : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_ReadSemantics( HNODE nodeidx );
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_ReadSemantics( *this ); };

	void SendVSSetup( SETUP_HLSL_VS_Input s );

	int GetTexcoordFlag( int numCoord );

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

	SETUP_HLSL_VS_Input vs_setup;
};



#endif