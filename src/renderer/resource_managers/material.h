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
			textureHandles[i] = INVALID_RESOURCE_HANDLE;
	}

	ShaderTextureFlags textures = ST_NONE;
	//48b padding
	resourceHandle shader;
	static const size_t MAX_TEXTURES = 2;
	resourceHandle textureHandles[MAX_TEXTURES];
	//Uniforms?
	//commandBuffer*
};


}