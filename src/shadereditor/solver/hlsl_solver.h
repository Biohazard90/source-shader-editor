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

#include "chlsl_solverbase.h"
#include "chlsl_solver_dummy.h"
#include "chlsl_solver_readsemantics.h"
#include "chlsl_solver_writesemantics.h"

#include "chlsl_solver_matrices.h"

#include "chlsl_solver_add.h"
#include "chlsl_solver_subtract.h"
#include "chlsl_solver_multiply.h"
#include "chlsl_solver_divide.h"

#include "chlsl_solver_swizzle.h"
#include "chlsl_solver_constant.h"
#include "chlsl_solver_dot.h"
#include "chlsl_solver_cross.h"
#include "chlsl_solver_vecref.h"
#include "chlsl_solver_baseevaluate.h"
#include "chlsl_solver_lerp.h"
#include "chlsl_solver_baserange.h"
#include "chlsl_solver_append.h"
#include "chlsl_solver_mcomp.h"

#include "chlsl_solver_callback.h"
#include "chlsl_solver_vmtparam.h"
#include "chlsl_solver_array.h"
#include "chlsl_solver_break.h"
#include "chlsl_solver_custom.h"

#include "chlsl_solver_texturesample.h"
#include "chlsl_solver_textransform.h"
#include "chlsl_solver_flashlight.h"
#include "chlsl_solver_lightscale.h"

#include "chlsl_solver_envc.h"
#include "chlsl_solver_containerbasic.h"
#include "chlsl_solver_utility.h"

#include "chlsl_solver_stdvlight.h"
#include "chlsl_solver_stdplight.h"
#include "chlsl_solver_stdskinning.h"
#include "chlsl_solver_vcompression.h"
#include "chlsl_solver_parallax.h"

#include "chlsl_solver_fog.h"
#include "chlsl_solver_final.h"


#include "chlsl_solverpp_base.h"
#include "chlsl_solver_pp_rt.h"
#include "chlsl_solver_pp_mat.h"
#include "chlsl_solver_pp_operations.h"
#include "chlsl_solver_pp_drawmat.h"
#include "chlsl_solver_pp_renderview.h"

#endif
