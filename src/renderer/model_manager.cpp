#include "model_manager.h"

#include "core/file/blob.h"
#include "core/file/clob.h"

#include <bgfx/bgfx.h>


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


struct PosColorVertex
{
	float x;
	float y;
	float z;
	u32 abgr;
	float u0;
	float v0;
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
	InputClob dataText(data);

	Mesh mesh;

	mesh.m_vertex_decl
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.end();

	int verticesCount;
	ASSERT(dataText.Read(verticesCount));

	u32 verticesBufferSize = verticesCount * sizeof(PosColorVertex);
	PosColorVertex* vertices = (PosColorVertex*)m_allocator.Allocate(verticesBufferSize, ALIGN_OF(PosColorVertex));
	for(int i = 0; i < verticesCount; ++i)
	{
		ASSERT(dataText.Read(vertices[i].x));
		dataText.Skip(1);
		ASSERT(dataText.Read(vertices[i].y));
		dataText.Skip(1);
		ASSERT(dataText.Read(vertices[i].z));
		dataText.Skip(1);
		ASSERT(dataText.Read(vertices[i].abgr));
		dataText.Skip(1);
		ASSERT(dataText.Read(vertices[i].u0));
		dataText.Skip(1);
		ASSERT(dataText.Read(vertices[i].v0));
	}

	int indicesCount;
	ASSERT(dataText.Read(indicesCount));
	indicesCount *= 3;

	u32 indicesBufferSize = indicesCount * sizeof(u16);
	u16* indices = (u16*)m_allocator.Allocate(indicesBufferSize, ALIGN_OF(u16));
	for(int i = 0; i < indicesCount; ++i)
	{
		int num;
		ASSERT(dataText.Read(num));
		indices[i] = (u16)num;
	}

	mesh.vertexBufferHandle = bgfx::createVertexBuffer(bgfx::copy(vertices, verticesBufferSize), mesh.m_vertex_decl);
	mesh.indexBufferHandle = bgfx::createIndexBuffer(bgfx::copy(indices, indicesBufferSize));

	m_allocator.Deallocate(vertices);
	m_allocator.Deallocate(indices);

	char materialPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(dataText.ReadLine(materialPath, Path::MAX_LENGTH));
	mesh.material = static_cast<materialHandle>(m_depManager->LoadResource(ResourceType::Model, ResourceType::Material, Path(materialPath)));

	model->meshes.PushBack(mesh);
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