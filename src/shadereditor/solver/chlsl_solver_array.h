#ifndef CHLSL_SOLVER_ARRAY_H
#define CHLSL_SOLVER_ARRAY_H

#include "editorcommon.h"

class CHLSL_Solver_Array : public CHLSL_SolverBase
{
public:
	CHLSL_Solver_Array( HNODE nodeidx );
	CHLSL_SolverBase *Copy(){
		return new CHLSL_Solver_Array( *this ); };
	CHLSL_Solver_Array( const CHLSL_Solver_Array &o );
	~CHLSL_Solver_Array();

	void SetState( int iComps, int iSize_x, int iSize_y,
					Vector4D *vecData );

protected:
	virtual void OnVarInit( const WriteContext_FXC &context );
	virtual void OnVarInit_PostStep();
	virtual void OnIdentifierAlloc( IdentifierLists_t &List );
	virtual void Render( Preview2DContext &c );

	int m_iNumComps;
	int m_iSize_x;
	int m_iSize_y;
	Vector4D *m_vecData;
};

#endif