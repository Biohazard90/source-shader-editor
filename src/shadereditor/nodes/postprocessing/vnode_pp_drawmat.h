#ifndef CNODE_PP_DRAWMAT_H
#define CNODE_PP_DRAWMAT_H

#include <editorCommon.h>

class CNodePP_DrawMat : public CNodePP_Base
{
	DECLARE_CLASS( CNodePP_DrawMat, CNodePP_Base );
public:

	enum
	{
		VPSIZINGMODE_USE_RT = -1,
		VPSIZINGMODE_FULL_FB = 0,
		VPSIZINGMODE_HALF_FB,
		VPSIZINGMODE_QUARTER_FB,
	};

	CNodePP_DrawMat( CNodeView *p );
	~CNodePP_DrawMat();

	virtual int GetNodeType(){ return HLSLNODE_POSTPROCESSING_DRAW_MATERIAL; };

	virtual int UpdateInputsValid();

	virtual KeyValues *AllocateKeyValues( int NodeIndex );
	virtual void RestoreFromKeyValues_Specific( KeyValues *pKV );

private:
	virtual bool CreateSolvers(GenericShaderData *ShaderData);

	int m_iSizingMode_Tg;
	int m_iSizingMode_Src;
	bool m_bPushTarget;
	vp_offsets_t m_offsets;
};


#endif
