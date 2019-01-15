#pragma once

#include "core/resource/resource.h"
#include "shader_inputs.h"


namespace Veng
{

enum class shaderInternalRenderHandle : u64 {};
const shaderInternalRenderHandle INVALID_SHADER_INTERNAL_RENDER_HANDLE = (shaderInternalRenderHandle)0;


struct ShaderInternal : public Resource
{
	ShaderInternal() : Resource(ResourceType::ShaderInternal) {}

	shaderInternalRenderHandle renderDataHandle;
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