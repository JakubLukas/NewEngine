#pragma once

#include "core/resource/resource.h"

#include "shader.h"
#include "texture.h"


namespace Veng
{


struct Material : public Resource
{
	Material()
		: Resource(ResourceType::Material)
	{
		for (size_t i = 0; i < Material::MAX_TEXTURES; ++i)
			textures[i] = INVALID_RESOURCE_HANDLE;
	}

	resourceHandle shader;
	static const size_t MAX_TEXTURES = 4;
	resourceHandle textures[MAX_TEXTURES];
	//Uniforms?
	//commandBuffer*
};


}