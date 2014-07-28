#ifndef HLSLSOLVER_H
#define HLSLSOLVER_H

enum ResourceType_t
{
	RESOURCETYPE_VARIABLE = 0,

	RESOURCETYPE_POS_3,
	RESOURCETYPE_POS_4,

	RESOURCETYPE_NORMAL,
	RESOURCETYPE_NORMAL_4,
	RESOURCETYPE_TANGENTS_3,
	RESOURCETYPE_TANGENTS_4,
	RESOURCETYPE_TANGENTT,

	RESOURCETYPE_FLEXDELTA,
	RESOURCETYPE_FLEXDELTA_NORMAL,
	RESOURCETYPE_BONE_WEIGHTS,
	RESOURCETYPE_BONE_INDICES,

	RESOURCETYPE_TEXCOORD_0,
	RESOURCETYPE_TEXCOORD_1,
	RESOURCETYPE_TEXCOORD_2,
	RESOURCETYPE_TEXCOORD_3,
	RESOURCETYPE_TEXCOORD_4,
	RESOURCETYPE_TEXCOORD_5,
	RESOURCETYPE_TEXCOORD_6,
	RESOURCETYPE_TEXCOORD_7,

	RESOURCETYPE_COLOR_0,
	RESOURCETYPE_COLOR_1,
	RESOURCETYPE_COLOR_2,
	RESOURCETYPE_COLOR_3,

	RESOURCETYPE_DEPTH,

	RESOURCETYPE_DUMMY,
	RESOURCETYPE_,
};

#include "cHLSL_SolverBase.h"
#include "cHLSL_Solver_Dummy.h"
#include "cHLSL_Solver_ReadSemantics.h"
#include "cHLSL_Solver_WriteSemantics.h"

#include "cHLSL_Solver_Matrices.h"

#include "cHLSL_Solver_Add.h"
#include "cHLSL_Solver_Subtract.h"
#include "cHLSL_Solver_Multiply.h"
#include "cHLSL_Solver_Divide.h"

#include "cHLSL_Solver_Swizzle.h"
#include "cHLSL_Solver_Constant.h"
#include "cHLSL_Solver_Dot.h"
#include "cHLSL_Solver_Cross.h"
#include "cHLSL_Solver_VecRef.h"
#include "cHLSL_Solver_BaseEvaluate.h"
#include "cHLSL_Solver_Lerp.h"
#include "cHLSL_Solver_BaseRange.h"
#include "cHLSL_Solver_Append.h"
#include "cHLSL_Solver_MComp.h"

#include "cHLSL_Solver_Callback.h"
#include "cHLSL_Solver_VmtParam.h"
#include "cHLSL_Solver_Array.h"
#include "cHLSL_Solver_Break.h"
#include "cHLSL_Solver_Custom.h"

#include "cHLSL_Solver_TextureSample.h"
#include "cHLSL_Solver_TexTransform.h"
#include "cHLSL_Solver_Flashlight.h"
#include "cHLSL_Solver_Lightscale.h"

#include "cHLSL_Solver_EnvC.h"
#include "cHLSL_Solver_ContainerBasic.h"
#include "cHLSL_Solver_Utility.h"

#include "cHLSL_Solver_StdVLight.h"
#include "cHLSL_Solver_StdPLight.h"
#include "cHLSL_Solver_StdSkinning.h"
#include "cHLSL_Solver_VCompression.h"
#include "cHLSL_Solver_Parallax.h"

#include "cHLSL_Solver_Fog.h"
#include "cHLSL_Solver_Final.h"


#include "cHLSL_SolverPP_Base.h"
#include "cHLSL_Solver_PP_RT.h"
#include "cHLSL_Solver_PP_Mat.h"
#include "cHLSL_Solver_PP_Operations.h"
#include "cHLSL_Solver_PP_DrawMat.h"
#include "cHLSL_Solver_PP_RenderView.h"

#endif