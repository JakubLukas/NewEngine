#pragma once

#include "core/resource/resource.h"


namespace Veng
{

enum class shaderInternalHandle : u64 {};
const shaderInternalHandle INVALID_SHADER_INTERNAL_HANDLE = (shaderInternalHandle)0;

enum class shaderInternalRenderHandle : u64 {};
const shaderInternalRenderHandle INVALID_SHADER_INTERNAL_RENDER_HANDLE = (shaderInternalRenderHandle)0;


struct ShaderInternal : public Resource
{
	ShaderInternal() : Resource(ResourceType::ShaderInternal) {}

	shaderInternalRenderHandle renderDataHandle;
};



enum class shaderHandle : u64 {};
const shaderHandle INVALID_SHADER_HANDLE = (shaderHandle)0;

enum class shaderRenderHandle : u64 {};
const shaderRenderHandle INVALID_SHADER_RENDER_HANDLE = (shaderRenderHandle)0;


struct Shader : public Resource
{
	Shader() : Resource(ResourceType::Shader) {}

	shaderInternalHandle vsHandle;
	shaderInternalHandle fsHandle;
	shaderRenderHandle renderDataHandle;
};


}