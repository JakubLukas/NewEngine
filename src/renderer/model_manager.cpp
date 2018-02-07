#include "model_manager.h"

#include <bgfx/bgfx.h>


namespace Veng
{


struct PosColorVertex
{
	float m_x;
	float m_y;
	float m_z;
	u32 m_abgr;
};


ModelManager::ModelManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
{

}


ModelManager::~ModelManager()
{

}


modelHandle ModelManager::Load(const Path& path)
{
	return ResourceManager::Load<modelHandle>(path);
}


void ModelManager::Unload(modelHandle handle)
{
	ResourceManager::Unload(handle);
}


void ModelManager::Reload(modelHandle handle)
{
	ResourceManager::Reload(handle);
}


const Model* ModelManager::GetResource(modelHandle handle) const
{
	return ResourceManager::GetResource<Model>(handle);
}


Resource* ModelManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Model)(m_allocator);
}


void ModelManager::DestroyResource(Resource* resource)
{
	Model* model = static_cast<Model*>(resource);

	for (Mesh& mesh : model->meshes)
	{
		bgfx::destroy(mesh.vertexBufferHandle);
		bgfx::destroy(mesh.indexBufferHandle);
		m_depManager->UnloadResource(ResourceType::Material, mesh.material);
	}

	DELETE_OBJECT(m_allocator, model);
}


void ModelManager::ReloadResource(Resource* resource)
{

}


void ModelManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Model* model = ResourceManager::GetResource<Model>(handle);

	Mesh mesh;

	mesh.m_vertex_decl
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();

	int verticesCount;
	ASSERT(data.Read(verticesCount));

	u32 verticesBufferSize = verticesCount * sizeof(PosColorVertex);
	PosColorVertex* vertices = (PosColorVertex*)m_allocator.Allocate(verticesBufferSize, ALIGN_OF(PosColorVertex));
	for(int i = 0; i < verticesCount; ++i)
	{
		ASSERT(data.Read(vertices[i].m_x));
		data.Skip(1);
		ASSERT(data.Read(vertices[i].m_y));
		data.Skip(1);
		ASSERT(data.Read(vertices[i].m_z));
		data.Skip(1);
		ASSERT(data.ReadHex(vertices[i].m_abgr));
	}

	int indicesCount;
	ASSERT(data.Read(indicesCount));
	indicesCount *= 3;

	u32 indicesBufferSize = indicesCount * sizeof(u16);
	u16* indices = (u16*)m_allocator.Allocate(indicesBufferSize, ALIGN_OF(u16));
	for(int i = 0; i < indicesCount; ++i)
	{
		int num;
		ASSERT(data.Read(num));
		indices[i] = (u16)num;
	}

	mesh.vertexBufferHandle = bgfx::createVertexBuffer(bgfx::copy(vertices, verticesBufferSize), mesh.m_vertex_decl);
	mesh.indexBufferHandle = bgfx::createIndexBuffer(bgfx::copy(indices, indicesBufferSize));

	m_allocator.Deallocate(vertices);
	m_allocator.Deallocate(indices);

	char materialPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadLine(materialPath, Path::MAX_LENGTH));
	mesh.material = m_depManager->LoadResource<materialHandle>(ResourceType::Model, ResourceType::Material, Path(materialPath));

	model->meshes.Push(mesh);
}


void ModelManager::ChildResourceLoaded(resourceHandle childResource)
{
	materialHandle childHandle = static_cast<materialHandle>(childResource);

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
	m_depManager->ResourceLoaded(ResourceType::Model, GetResourceHandle<resourceHandle>(model));
}


}