#ifndef CHLSL_SOLVER_PARALLAX_H
#define CHLSL_SOLVER_PARALLAX_H

#include "editorcommon.h"

struct ParallaxConfig_t
{
public:
	enum
	{
		PCFG_POM = 0,
		PCFG_RELIEF,
	};

	ParallaxConfig_t()
	{
		iParallaxMode = PCFG_POM;
		bGradientLookup = true;

		iSamplesMin = 8;
		iSamplesMax = 42;
		iBinaryMax = 10;

		bGetOffset = false;
	};

	int iParallaxMode;
	bool bGradientLookup;

	int iSamplesMin;
	int iSamplesMax;
	int iBinaryMax;

	bool bGetOffset;
};

class CHLSL_Solver_Parallax : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Parallax( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Parallax( *this ); };

	void SetState( ParallaxConfig_t data );

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

	ParallaxConfig_t m_PSetup;
};

class CHLSL_Solver_Parallax_StdShadow : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Parallax_StdShadow( HNODE nodeidx ) : CHLSL_SolverBase( nodeidx ) {};
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Parallax_StdShadow( *this ); };

protected:
	virtual void OnWriteFXC( bool bIsPixelShader, WriteContext_FXC &context );
	virtual void Render( Preview2DContext &c );

};

#endif