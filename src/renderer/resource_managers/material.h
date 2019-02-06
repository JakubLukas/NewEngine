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
	{}

	static const size_t MAX_TEXTURES = 4;

	resourceHandle shader;
	resourceHandle textures[MAX_TEXTURES];
	u8 textureCount = 0;
	//8b padding
	materialRenderHandle renderDataHandle = INVALID_MATERIAL_RENDER_HANDLE;
	//Uniforms?
	//commandBuffer*
};


}