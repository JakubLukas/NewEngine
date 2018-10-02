#include "texture_manager.h"

#include "core/file/blob.h"
#include "core/memory.h"

#define STBI_NO_STDIO
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC
#define STBI_REALLOC
#define STBI_FREE
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

}


TextureManager::~TextureManager()
{

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

	m_allocator.Deallocate(texture->data);

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
	const u8* data = (const u8*)stbi_load_from_memory((const stbi_uc*)data.GetData(), data.GetSize(), &x, &y, &channels, 0);

	texture->width = 64;//////////////////////////
	texture->height = 64;/////////////////////////
	texture->data = (u8*)m_allocator.Allocate(data.GetSize(), ALIGN_OF(u8));
	memory::Copy(texture->data, data.GetData(), data.GetSize());
}


}