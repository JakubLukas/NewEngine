#include "texture_manager.h"


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
	Texture* material = static_cast<Texture*>(resource);

	m_allocator.Deallocate(material->data);

	DELETE_OBJECT(m_allocator, material);
}


void TextureManager::ReloadResource(Resource* resource)
{
	ASSERT2(false, "Not implemented yet");
}


void TextureManager::ResourceLoaded(resourceHandle handle, InputClob& data)
{
	Material* material = static_cast<Material*>(ResourceManager::GetResource(handle));

	char shaderPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadString(shaderPath, Path::MAX_LENGTH));
	material->shader = static_cast<shaderHandle>(m_depManager->LoadResource(ResourceType::Material, ResourceType::Shader, Path(shaderPath)));
}


void TextureManager::ChildResourceLoaded(resourceHandle childResource)
{
	shaderHandle childHandle = static_cast<shaderHandle>(childResource);

	for(auto& res : m_resources)
	{
		Material* material = static_cast<Material*>(res.value);
		if(material->shader == childHandle)
		{
			FinalizeMaterial(material);
		}
	}
}


void TextureManager::FinalizeMaterial(Material* material)
{
	material->SetState(Resource::State::Ready);
	m_depManager->ResourceLoaded(ResourceType::Material, GetResourceHandle(material));
}


}