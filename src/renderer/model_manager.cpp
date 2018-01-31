#include "model_manager.h"


namespace Veng
{


ModelManager::ModelManager(IAllocator& allocator, FileSystem& fileSystem, MaterialManager* materialManager)
	: ResourceManager(allocator, fileSystem)
	, m_materialManager(materialManager)
{

}


ModelManager::~ModelManager()
{

}


modelHandle ModelManager::Load(const Path& path)
{
	return static_cast<modelHandle>(LoadInternal(path));
}


void ModelManager::Unload(modelHandle handle)
{
	UnloadInternal(static_cast<resourceHandle>(handle));
}


void ModelManager::Reload(modelHandle handle)
{
	ReloadInternal(static_cast<resourceHandle>(handle));
}


const Model* ModelManager::GetResource(modelHandle handle) const
{
	return static_cast<Model*>(ResourceManager::GetResource(static_cast<resourceHandle>(handle)));
}


Resource* ModelManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Model)(m_allocator);
}


void ModelManager::DestroyResource(Resource* resource)
{
	Model* material = static_cast<Model*>(resource);

	for (Mesh& mesh : material->meshes)
	{
		mesh.Clear();
		m_materialManager->Unload(mesh.material);
	}

	DELETE_OBJECT(m_allocator, material);
}


void ModelManager::ReloadResource(Resource* resource)
{

}


bool ModelManager::ResourceLoaded(Resource* resource, InputBlob& data)
{
	Model* material = static_cast<Model*>(resource);

	/*char modelPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadString(shaderPath, Path::MAX_LENGTH));
	material->shader = m_shaderManager->Load(Path(shaderPath));
	m_shaderManager->AddDependency(resource, static_cast<resourceHandle>(material->shader));*/

	return true;
}


void ModelManager::ChildResourceLoaded(resourceHandle childResource)
{
	shaderHandle childHandle = static_cast<shaderHandle>(childResource);

	/*for (auto& res : m_resources)
	{
		Model* model = static_cast<Model*>(res.value);
		if (material->shader == childHandle)
		{
			FinalizeModel(material);
		}
	}*/
}


void ModelManager::FinalizeModel(Model* material)
{
	material->SetState(Resource::State::Ready);
}


}