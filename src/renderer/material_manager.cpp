#include "material_manager.h"


namespace Veng
{


MaterialManager::MaterialManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{

}


MaterialManager::~MaterialManager()
{

}


materialHandle MaterialManager::Load(const Path& path)
{
	return ResourceManager::Load<materialHandle>(path);
}


void MaterialManager::Unload(materialHandle handle)
{
	ResourceManager::Unload(handle);
}


void MaterialManager::Reload(materialHandle handle)
{
	ResourceManager::Reload(handle);
}


const Material* MaterialManager::GetResource(materialHandle handle) const
{
	return ResourceManager::GetResource<Material>(handle);
}


Resource* MaterialManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Material)();
}


void MaterialManager::DestroyResource(Resource* resource)
{
	Material* material = static_cast<Material*>(resource);

	m_depManager->UnloadResource(ResourceType::Shader, material->shader);

	DELETE_OBJECT(m_allocator, material);
}


void MaterialManager::ReloadResource(Resource* resource)
{
	
}


void MaterialManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Material* material = ResourceManager::GetResource<Material>(handle);

	char shaderPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadString(shaderPath, Path::MAX_LENGTH));
	material->shader = m_depManager->LoadResource<shaderHandle>(ResourceType::Material, ResourceType::Shader, Path(shaderPath));
}


void MaterialManager::ChildResourceLoaded(resourceHandle childResource)
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


void MaterialManager::FinalizeMaterial(Material* material)
{
	material->SetState(Resource::State::Ready);
	m_depManager->ResourceLoaded(ResourceType::Material, GetResourceHandle<resourceHandle>(material));
}


}