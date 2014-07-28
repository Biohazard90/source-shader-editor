#ifndef CHLSL_SOLVER_SWIZZLE_H
#define CHLSL_SOLVER_SWIZZLE_H

#include "editorcommon.h"

class CHLSL_Solver_Swizzle : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Swizzle( HNODE nodeidx );
	CHLSL_SolverBase *Copy();
	CHLSL_Solver_Swizzle( const CHLSL_Solver_Swizzle& o );

	void SetComponentInfo( const TokenChannels_t &info );
	//void SetTargetSize( int s );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

	//int iCompOrder[4];
	//int targetSize;

	TokenChannels_t m_SwizzleData;
};

#endif