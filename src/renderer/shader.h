#pragma once

#include "core/resource/resource.h"


namespace Veng
{

enum class shaderHandle : u64 {};
const shaderHandle INVALID_SHADER_HANDLE = (shaderHandle)0;


struct Shader : public Resource
{
	Shader() : Resource(ResourceType::Shader) {}

	//char* fileContent = nullptr;
	shaderInternalHandle vsHandle;
	shaderInternalHandle fsHandle;
	ShaderProgramInternal program;
	bool vsLoaded = false;
	bool fsLoaded = false;
};


}