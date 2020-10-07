#include "texture_manager.h"

#include "core/file/blob.h"
#include "core/memory.h"
#include "core/allocators.h"
#include "core/asserts.h"

#include "../renderer.h"


static Veng::ProxyAllocator* stbImageAllocator = nullptr;

static void* stbImageAlloc(size_t size)
{
	return stbImageAllocator->Allocate(size, alignof(Veng::u8));
}

static void* stbImageRealloc(void* ptr, size_t size)
{
	return stbImageAllocator->Reallocate(ptr, size, alignof(Veng::u8));
}

static void stbImageDealloc(void* ptr)
{
	stbImageAllocator->Deallocate(ptr);
}

#define STBI_MALLOC stbImageAlloc
#define STBI_REALLOC stbImageRealloc
#define STBI_FREE stbImageDealloc
#define STBI_ASSERT(x) ASSERT(x)
#define STBI_NO_STDIO
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"


namespace Veng
{


ResourceType Texture::RESOURCE_TYPE("texture");


TextureManager::TextureManager(Allocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(Texture::RESOURCE_TYPE, allocator, fileSystem, depManager)
{
	stbImageAllocator = NEW_OBJECT(m_allocator, ProxyAllocator)(m_allocator);
	stbImageAllocator->SetDebugName("stb image");
}


TextureManager::~TextureManager()
{
	DELETE_OBJECT(m_allocator, stbImageAllocator);
}


const char* const * TextureManager::GetSupportedFileExt() const
{
	static const char* buffer[] = { "tga" };
	return buffer;
}

size_t TextureManager::GetSupportedFileExtCount() const
{
	return 1;
}


void TextureManager::SetRenderSystem(RenderSystem* renderSystem)
{
	m_renderSystem = renderSystem;
}


Resource* TextureManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Texture)();
}


void TextureManager::DestroyResource(Resource* resource)
{
	Texture* texture = static_cast<Texture*>(resource);

	m_renderSystem->DestroyTextureData(texture->renderDataHandle);

	stbi_image_free(texture->data);

	DELETE_OBJECT(m_allocator, texture);
}


void TextureManager::ReloadResource(Resource* resource)
{
	ASSERT2(false, "Not implemented yet");
}


void TextureManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Texture* texture = static_cast<Texture*>(GetResource(handle));

	int x;
	int y;
	int channels;
	u8* imageData = (u8*)stbi_load_from_memory((const stbi_uc*)data.GetData(), (int)data.GetSize(), &x, &y, &channels, 0);

	ASSERT2(imageData != nullptr, "Invalid image");

	texture->width = (u32)x;
	texture->height = (u32)y;
	texture->channels = (u32)channels;
	texture->data = imageData;

	texture->renderDataHandle = m_renderSystem->CreateTextureData(*texture);

	texture->SetState(Resource::State::Ready);
	m_depManager->ResourceLoaded(Texture::RESOURCE_TYPE, GetResourceHandle(texture));
}


}