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
	static ResourceType RESOURCE_TYPE;

	Material() : Resource(RESOURCE_TYPE) {}

	static const u8 MAX_UNIFORMS = 4;
	static const u8 MAX_TEXTURES = 4;

	resourceHandle shader;
	resourceHandle textures[MAX_TEXTURES];
	ShaderUniform outputUniforms[MAX_UNIFORMS];//TODO: separate, just for editor
	ShaderTexture outputTextures[MAX_UNIFORMS];//TODO: separate, just for editor
	u8 outputUniformCount = 0;//TODO: separate, just for editor
	u8 textureCount = 0;
	//8b padding
	materialRenderHandle renderDataHandle = INVALID_MATERIAL_RENDER_HANDLE;
	//Uniforms?
	//commandBuffer*
};


}