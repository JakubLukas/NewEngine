#pragma once

#include "core/resource/resource.h"


namespace Veng
{

enum class shaderInternalRenderHandle : u64 {};
const shaderInternalRenderHandle INVALID_SHADER_INTERNAL_RENDER_HANDLE = (shaderInternalRenderHandle)0;


struct ShaderInternal : public Resource
{
	ShaderInternal() : Resource(ResourceType::ShaderInternal) {}

	shaderInternalRenderHandle renderDataHandle;
};



typedef Veng::u32 ShaderVaryingFlags;
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

typedef Veng::u16 ShaderUniformFlags;
enum ShaderUniformBits : ShaderUniformFlags
{
	SU_NONE = 0,
};

typedef Veng::u8 ShaderTextureFlags;
enum ShaderTextureBits : ShaderTextureFlags
{
	ST_NONE = 0,
	ST_DIFF_TEXTURE_BIT = 1 << 0,
	ST_NORM_TEXTURE_BIT = 1 << 1,
};



enum class shaderRenderHandle : u64 {};
const shaderRenderHandle INVALID_SHADER_RENDER_HANDLE = (shaderRenderHandle)0;


struct Shader : public Resource
{
	Shader() : Resource(ResourceType::Shader) {}

	ShaderVaryingFlags varyings = SV_NONE;
	ShaderUniformFlags uniforms = SU_NONE;
	ShaderTextureFlags textures = ST_NONE;
	//8b padding, can be used
	resourceHandle vsHandle;
	resourceHandle fsHandle;
	shaderRenderHandle renderDataHandle;
};


}