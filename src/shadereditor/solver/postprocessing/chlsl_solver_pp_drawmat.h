#ifndef CHLSL_SOLVER_PP_DRAWMAT_H
#define CHLSL_SOLVER_PP_DRAWMAT_H

#include "editorcommon.h"

struct vp_offsets_t
{
	int dst_x, dst_y;
	int dst_w, dst_h;

	float src_x0, src_y0;
	float src_x1, src_y1;

	int src_w, src_h;
};

class CHLSL_Solver_PP_DrawMat : public CHLSL_Solver_PP_Base
{
public:
	CHLSL_Solver_PP_DrawMat( HNODE nodeidx );
	virtual CHLSL_Solver_PP_DrawMat *Copy(){
		return new CHLSL_Solver_PP_DrawMat( *this ); };

		void Init( int sizing_tg, int sizing_src, bool bPushTarget, vp_offsets_t offsets );

protected:
	virtual void OnExecuteCode( const RunCodeContext &context );

	bool m_bPushTarget;
	int m_iSizingMode_tg;
	int m_iSizingMode_src;

	vp_offsets_t m_offsets;
};

#endif