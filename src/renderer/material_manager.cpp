#include "material_manager.h"


namespace Veng
{


inline static materialHandle GenericToMaterialHandle(resourceHandle handle)
{
	return static_cast<materialHandle>(handle);
}

inline static resourceHandle MaterialToGenericHandle(materialHandle handle)
{
	return static_cast<resourceHandle>(handle);
}


MaterialManager::MaterialManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{

}


MaterialManager::~MaterialManager()
{

}


materialHandle MaterialManager::Load(const Path& path)
{
	return GenericToMaterialHandle(ResourceManager::Load(path));
}


void MaterialManager::Unload(materialHandle handle)
{
	ResourceManager::Unload(MaterialToGenericHandle(handle));
}


void MaterialManager::Reload(materialHandle handle)
{
	ResourceManager::Reload(MaterialToGenericHandle(handle));
}


const Material* MaterialManager::GetResource(materialHandle handle) const
{
	return static_cast<const Material*>(ResourceManager::GetResource(MaterialToGenericHandle(handle)));
}


Resource* MaterialManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Material)();
}


void MaterialManager::DestroyResource(Resource* resource)
{
	Material* material = static_cast<Material*>(resource);

	m_depManager->UnloadResource(ResourceType::Shader, static_cast<resourceHandle>(material->shader));

	DELETE_OBJECT(m_allocator, material);
}


void MaterialManager::ReloadResource(Resource* resource)
{
	
}


void MaterialManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Material* material = static_cast<Material*>(ResourceManager::GetResource(handle));

	char shaderPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadString(shaderPath, Path::MAX_LENGTH));
	material->shader = static_cast<shaderHandle>(m_depManager->LoadResource(ResourceType::Material, ResourceType::Shader, Path(shaderPath)));
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
	m_depManager->ResourceLoaded(ResourceType::Material, GetResourceHandle(material));
}


}