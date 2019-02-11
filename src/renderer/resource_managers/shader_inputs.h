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

struct ShaderUniform
{
	enum class Type : u8
	{
		Float
	};
	Type type;
	static const u8 MAX_NAME_LENGTH = 15;
	char name[MAX_NAME_LENGTH + 1] = { 0 };
	u8 count;
};

struct ShaderTexture
{
	static const u8 MAX_NAME_LENGTH = 15;
	char name[MAX_NAME_LENGTH + 1] = { 0 };
};


}