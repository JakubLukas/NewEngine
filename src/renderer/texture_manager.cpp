#include "texture_manager.h"

#include "core/file/blob.h"
#include "core/memory.h"
#include "core/allocators.h"


static Veng::HeapAllocator* stbImageAllocator = nullptr;

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
#define STBI_NO_STDIO
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"


namespace Veng
{


inline static textureHandle GenericToMaterialHandle(resourceHandle handle)
{
	return static_cast<textureHandle>(handle);
}

inline static resourceHandle MaterialToGenericHandle(textureHandle handle)
{
	return static_cast<resourceHandle>(handle);
}


TextureManager::TextureManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{
	stbImageAllocator = NEW_OBJECT(m_allocator, HeapAllocator)(m_allocator);
	stbImageAllocator->SetDebugName("stb image");
}


TextureManager::~TextureManager()
{
	DELETE_OBJECT(m_allocator, stbImageAllocator);
}


textureHandle TextureManager::Load(const Path& path)
{
	return GenericToMaterialHandle(ResourceManager::Load(path));
}


void TextureManager::Unload(textureHandle handle)
{
	ResourceManager::Unload(MaterialToGenericHandle(handle));
}


void TextureManager::Reload(textureHandle handle)
{
	ResourceManager::Reload(MaterialToGenericHandle(handle));
}


const Texture* TextureManager::GetResource(textureHandle handle) const
{
	return static_cast<const Texture*>(ResourceManager::GetResource(MaterialToGenericHandle(handle)));
}


Resource* TextureManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Texture)();
}


void TextureManager::DestroyResource(Resource* resource)
{
	Texture* texture = static_cast<Texture*>(resource);

	stbi_image_free(texture->data);

	DELETE_OBJECT(m_allocator, texture);
}


void TextureManager::ReloadResource(Resource* resource)
{
	ASSERT2(false, "Not implemented yet");
}


void TextureManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Texture* texture = static_cast<Texture*>(ResourceManager::GetResource(handle));

	int x;
	int y;
	int channels;
	u8* imageData = (u8*)stbi_load_from_memory((const stbi_uc*)data.GetData(), (int)data.GetSize(), &x, &y, &channels, 0);

	ASSERT2(imageData != nullptr, "Invalid image");

	texture->width = (u32)x;
	texture->height = (u32)y;
	texture->channels = (u32)channels;
	texture->data = imageData;

	const bgfx::Memory* mem = bgfx::makeRef(texture->data, texture->width * texture->height * texture->channels);

	texture->handle = bgfx::createTexture2D(
		uint16_t(texture->width)
		, uint16_t(texture->height)
		, false //hasMipMap
		, 1
		, bgfx::TextureFormat::Enum::RGBA8
		, BGFX_TEXTURE_NONE
		, mem
	);

	ASSERT(bgfx::isValid(texture->handle));
	//if(bgfx::isValid(texture->handle))
	{
		//bgfx::setName(texture->handle, _filePath);
	}

	texture->SetState(Resource::State::Ready);
	m_depManager->ResourceLoaded(ResourceType::Texture, GetResourceHandle(texture));
}


}