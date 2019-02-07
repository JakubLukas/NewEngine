#pragma once

#include "core/resource/resource.h"
#include "shader_inputs.h"


namespace Veng
{

enum class shaderInternalRenderHandle : u32 {};
const shaderInternalRenderHandle INVALID_SHADER_INTERNAL_RENDER_HANDLE = (shaderInternalRenderHandle)0;


struct ShaderInternal : public Resource
{
	ShaderInternal() : Resource(ResourceType::ShaderInternal) {}

	shaderInternalRenderHandle renderDataHandle;
};


enum class shaderRenderHandle : u32 {};
const shaderRenderHandle INVALID_SHADER_RENDER_HANDLE = (shaderRenderHandle)0;


struct Shader : public Resource
{
	Shader() : Resource(ResourceType::Shader) {}

	static const u8 MAX_UNIFORMS = 4;
	static const u8 MAX_TEXTURES = 4;

	ShaderVaryingFlags varyings = ShaderVarying_None;//TODO: separate, just for editor
	u8 inputUniformCount = 0;//TODO: separate, just for editor
	u8 inputTextureCount = 0;//TODO: separate, just for editor
	//16b padding
	ShaderUniform inputUniforms[MAX_UNIFORMS];//TODO: separate, just for editor
	ShaderUniform inputTextures[MAX_TEXTURES];//TODO: separate, just for editor
	u8 inputDirectionalLightsCount = 0;//TODO: separate, just for editor
	u8 inputPointLightsCount = 0;//TODO: separate, just for editor
	resourceHandle vsHandle;
	resourceHandle fsHandle;
	shaderRenderHandle renderDataHandle;
};


}