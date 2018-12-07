#pragma once

#include "core/resource/resource.h"

#include "shader.h"
#include "texture.h"


namespace Veng
{

enum class materialHandle : u64 {};
const materialHandle INVALID_MATERIAL_HANDLE = (materialHandle)0;


struct Material : public Resource
{
	Material()
		: Resource(ResourceType::Material)
	{
		for (size_t i = 0; i < Material::MAX_TEXTURES; ++i)
			textures[i] = (textureHandle)INVALID_HANDLE;
	}

	shaderHandle shader;
	static const size_t MAX_TEXTURES = 4;
	textureHandle textures[MAX_TEXTURES];
	//Uniforms?
	//commandBuffer*
};


}