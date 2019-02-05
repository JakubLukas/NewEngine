#pragma once

#include "core/resource/resource.h"

#include "shader.h"
#include "texture.h"


namespace Veng
{

enum class materialRenderHandle : u32 {};
const materialRenderHandle INVALID_MATERIAL_RENDER_HANDLE = (materialRenderHandle)0;


struct Material : public Resource
{
	Material()
		: Resource(ResourceType::Material)
	{
		for (size_t i = 0; i < Material::MAX_TEXTURES; ++i)
			textureHandles[i] = INVALID_RESOURCE_HANDLE;
	}

	static const size_t MAX_TEXTURES = 2;

	ShaderTextureFlags textures = ST_NONE;
	materialRenderHandle renderDataHandle = INVALID_MATERIAL_RENDER_HANDLE;
	resourceHandle shader;
	resourceHandle textureHandles[MAX_TEXTURES];
	//Uniforms?
	//commandBuffer*
};


}