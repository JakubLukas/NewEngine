#pragma once

#include "core/int.h"


namespace Veng
{


typedef u32 ShaderVaryingFlags;
enum ShaderVaryingBits : ShaderVaryingFlags
{
	ShaderVarying_None = 0,
	ShaderVarying_Position = 1 << 0,
	ShaderVarying_Color0 = 1 << 1,
	ShaderVarying_Texcoords0 = 1 << 2,
	ShaderVarying_Texcoords1 = 1 << 3,
	ShaderVarying_Normal = 1 << 4,
	ShaderVarying_Tangent = 1 << 5,
	ShaderVarying_Binormal = 1 << 6,
};

ShaderVaryingBits GetShaderVaryingFromString(const char* str);
const char* GetShaderVaryingName(ShaderVaryingBits value);



typedef u16 ShaderUniformFlags;
enum ShaderUniformBits : ShaderUniformFlags
{
	ShaderUniform_None = 0,
};

ShaderUniformBits GetShaderUniformFromString(const char* str);
const char* GetShaderUniformName(ShaderUniformBits value);



typedef u8 ShaderTextureFlags;
enum ShaderTextureBits : ShaderTextureFlags
{
	ShaderTexture_None = 0,
	ShaderTexture_Diffuse = 1 << 0,
	ShaderTexture_Normal = 1 << 1,
};

ShaderTextureBits GetShaderTextureFromString(const char* str);
const char* GetShaderTextureName(ShaderTextureBits value);


}