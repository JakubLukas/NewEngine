#pragma once

#include "core/resource/resource.h"


namespace Veng
{

enum class textureRenderHandle : u32 {};
const textureRenderHandle INVALID_TEXTURE_RENDER_HANDLE = (textureRenderHandle)0;


struct Texture : public Resource
{
	Texture() : Resource(ResourceType::Texture) {}

	u32 width = 0;
	u32 height = 0;
	u32 channels = 0;
	textureRenderHandle renderDataHandle = INVALID_TEXTURE_RENDER_HANDLE;
	u8* data = nullptr;
};


}