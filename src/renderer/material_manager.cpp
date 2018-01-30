#include "material_manager.h"


namespace Veng
{


MaterialManager::MaterialManager(IAllocator& allocator, FileSystem& fileSystem, ShaderManager* shaderManager)
	: ResourceManager(allocator, fileSystem)
	, m_shaderManager(shaderManager)
{

}


MaterialManager::~MaterialManager()
{

}


materialHandle MaterialManager::Load(const Path& path)
{
	return static_cast<materialHandle>(LoadInternal(path));
}


void MaterialManager::Unload(materialHandle handle)
{
	UnloadInternal(static_cast<resourceHandle>(handle));
}


void MaterialManager::Reload(materialHandle handle)
{
	ReloadInternal(static_cast<resourceHandle>(handle));
}


const Material* MaterialManager::GetResource(materialHandle handle) const
{
	return static_cast<Material*>(ResourceManager::GetResource(static_cast<resourceHandle>(handle)));
}


Resource* MaterialManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Material)();
}


void MaterialManager::DestroyResource(Resource* resource)
{
	Material* material = static_cast<Material*>(resource);

	m_shaderManager->Unload(material->shader);

	DELETE_OBJECT(m_allocator, material);
}


void MaterialManager::ReloadResource(Resource* resource)
{
	
}


bool MaterialManager::ResourceLoaded(Resource* resource, InputBlob& data)
{
	Material* material = static_cast<Material*>(resource);

	char shaderPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadString(shaderPath, Path::MAX_LENGTH));
	material->shader = m_shaderManager->Load(Path(shaderPath));
	m_shaderManager->AddDependency(resource, static_cast<resourceHandle>(material->shader));

	return true;
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
}


}