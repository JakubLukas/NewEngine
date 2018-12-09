#include "model_manager.h"

#include "core/file/blob.h"
#include "core/file/clob.h"

#include "renderer.h"


namespace Veng
{


inline static modelHandle GenericToModelHandle(resourceHandle handle)
{
	return static_cast<modelHandle>(handle);
}

inline static resourceHandle ModelToGenericHandle(modelHandle handle)
{
	return static_cast<resourceHandle>(handle);
}


ModelManager::ModelManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{}


ModelManager::~ModelManager()
{}


modelHandle ModelManager::Load(const Path& path)
{
	return GenericToModelHandle(ResourceManager::Load(path));
}


void ModelManager::Unload(modelHandle handle)
{
	ResourceManager::Unload(ModelToGenericHandle(handle));
}


void ModelManager::Reload(modelHandle handle)
{
	ResourceManager::Reload(ModelToGenericHandle(handle));
}


const Model* ModelManager::GetResource(modelHandle handle) const
{
	return static_cast<const Model*>(ResourceManager::GetResource(ModelToGenericHandle(handle)));
}


void ModelManager::SetRenderSystem(RenderSystem* renderSystem)
{
	m_renderSystem = renderSystem;
}


Resource* ModelManager::CreateResource()
{
	Resource* res = NEW_OBJECT(m_allocator, Model)(m_allocator);
	return res;
}


void ModelManager::DestroyResource(Resource* resource)
{
	Model* model = static_cast<Model*>(resource);

	for (Mesh& mesh : model->meshes)
	{
		m_renderSystem->DestroyMeshData(mesh.renderDataHandle);
		m_depManager->UnloadResource(ResourceType::Material, static_cast<resourceHandle>(mesh.material));
	}

	DELETE_OBJECT(m_allocator, model);
}


void ModelManager::ReloadResource(Resource* resource)
{
	ASSERT2(false, "Not implemented yet");
}


void ModelManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Model* model = static_cast<Model*>(ResourceManager::GetResource(handle));
	
	Mesh& mesh = model->meshes.PushBack();

	mesh.renderDataHandle = m_renderSystem->CreateMeshData(data);

	InputClob dataText(data);

	char materialPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(dataText.ReadLine(materialPath, Path::MAX_LENGTH));
	mesh.material = static_cast<materialHandle>(m_depManager->LoadResource(ResourceType::Model, ResourceType::Material, Path(materialPath)));
}


void ModelManager::ChildResourceLoaded(resourceHandle handle, ResourceType type)
{
	materialHandle childHandle = static_cast<materialHandle>(handle);

	for (auto& res : m_resources)
	{
		Model* model = static_cast<Model*>(res.value);
		if (model->meshes[0].material == childHandle)
		{
			FinalizeModel(model);
		}
	}
}


void ModelManager::FinalizeModel(Model* model)
{
	model->SetState(Resource::State::Ready);
	m_depManager->ResourceLoaded(ResourceType::Model, GetResourceHandle(model));
}


}