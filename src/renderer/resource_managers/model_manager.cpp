#include "model_manager.h"

#include "core/file/blob.h"
#include "core/parsing/json.h"

#include "../renderer.h"


namespace Veng
{


ModelManager::ModelManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{}


ModelManager::~ModelManager()
{}


const char* const * ModelManager::GetSupportedFileExt() const
{
	static const char* buffer[] = { "model" };
	return buffer;
}

size_t ModelManager::GetSupportedFileExtCount() const
{
	return 1;
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
	Model* model = static_cast<Model*>(GetResource(handle));
	
	Mesh& mesh = model->meshes.PushBack();

	mesh.renderDataHandle = m_renderSystem->CreateMeshData(data);

	char errorBuffer[64] = { 0 };
	JsonValue parsedJson;
	ASSERT(JsonParse((char*)data.GetData(), &m_allocator, &parsedJson, errorBuffer));
	ASSERT(JsonIsObject(&parsedJson));

	JsonKeyValue* material = JsonObjectFind(&parsedJson, "material");
	if(material != nullptr)
	{
		ASSERT(JsonIsString(&material->value));
		const char* materialRawStr = JsonGetString(&material->value);
		Path materialPath(materialRawStr);
		mesh.material = m_depManager->LoadResource(ResourceType::Model, ResourceType::Material, materialPath);
	}

	JsonDeinit(&parsedJson);
}


void ModelManager::ChildResourceLoaded(resourceHandle handle, ResourceType type)
{
	for (auto& res : m_resources)
	{
		Model* model = static_cast<Model*>(res.value);
		if (model->meshes[0].material == handle)
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