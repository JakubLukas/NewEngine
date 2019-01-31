#pragma once

#include "core/int.h"


namespace Veng
{


typedef u32 ShaderVaryingFlags;
enum ShaderVaryingBits : ShaderVaryingFlags
{
	SV_NONE = 0,
	SV_POSITION_BIT = 1 << 0,
	SV_COLOR0_BIT = 1 << 1,
	SV_TEXCOORDS0_BIT = 1 << 2,
	SV_TEXCOORDS1_BIT = 1 << 3,
	SV_NORMAL_BIT = 1 << 4,
	SV_TANGENT_BIT = 1 << 5,
	SV_BINORMAL_BIT = 1 << 6,
};

ShaderVaryingBits GetShaderVaryingFromString(const char* str);
const char* GetShaderVaryingName(ShaderVaryingBits value);



typedef u16 ShaderUniformFlags;
enum ShaderUniformBits : ShaderUniformFlags
{
	SU_NONE = 0,
};

ShaderUniformBits GetShaderUniformFromString(const char* str);
const char* GetShaderUniformName(ShaderUniformBits value);



typedef u8 ShaderTextureFlags;
enum ShaderTextureBits : ShaderTextureFlags
{
	ST_NONE = 0,
	ST_DIFF_TEXTURE_BIT = 1 << 0,
	ST_NORM_TEXTURE_BIT = 1 << 1,
};

ShaderTextureBits GetShaderTextureFromString(const char* str);
const char* GetShaderTextureName(ShaderTextureBits value);


}