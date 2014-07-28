#ifndef SHADERINCLUDES_H
#define SHADERINCLUDES_H

#ifdef SHADER_EDITOR_DLL_2006
#include "../../../shadereditor_dll_2006/materialsystem/stdshaders/BaseVSShader.h"

#define SHADER_SAMPLER0 SHADER_TEXTURE_STAGE0
#define SHADER_SAMPLER1 SHADER_TEXTURE_STAGE1
#define SHADER_SAMPLER2 SHADER_TEXTURE_STAGE2
#define SHADER_SAMPLER3 SHADER_TEXTURE_STAGE3
#define SHADER_SAMPLER4 SHADER_TEXTURE_STAGE4
#define SHADER_SAMPLER5 SHADER_TEXTURE_STAGE5
#define SHADER_SAMPLER6 SHADER_TEXTURE_STAGE6
#define SHADER_SAMPLER7 SHADER_TEXTURE_STAGE7
#define SHADER_SAMPLER8 SHADER_TEXTURE_STAGE8
#define SHADER_SAMPLER9 SHADER_TEXTURE_STAGE9
#define SHADER_SAMPLER10 SHADER_TEXTURE_STAGE10
#define SHADER_SAMPLER11 SHADER_TEXTURE_STAGE11
#define SHADER_SAMPLER12 SHADER_TEXTURE_STAGE12
#define SHADER_SAMPLER13 SHADER_TEXTURE_STAGE13
#define SHADER_SAMPLER14 SHADER_TEXTURE_STAGE14
#define SHADER_SAMPLER15 SHADER_TEXTURE_STAGE15

#define Sampler_t TextureStage_t

#define VERTEXSHADERVERTEXFORMAT( flags, coords, dims, userdatasize )\
	pShaderShadow->VertexShaderVertexFormat( flags | (IS_FLAG_SET(MATERIAL_VAR_MODEL)?VERTEX_BONE_INDEX:0),\
	coords, dims, IS_FLAG_SET(MATERIAL_VAR_MODEL)?3:0, userdatasize );

typedef int VertexCompressionType_t;

#else
#include "procshader/BaseVSShader.h"

#define VERTEXSHADERVERTEXFORMAT( flags, coords, dims, userdatasize )\
	pShaderShadow->VertexShaderVertexFormat( flags, coords, dims, userdatasize );
#endif

#ifdef SHADER_EDITOR_DLL_2006
#include "VMatrix.h"
#else
#include "mathlib/VMatrix.h"
#endif

#endif