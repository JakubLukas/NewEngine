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
	Model* model = static_cast<Model*>(resource);

	for (Mesh& mesh : model->meshes)
	{
		mesh.Clear();
		m_materialManager->Unload(mesh.material);
	}

	DELETE_OBJECT(m_allocator, model);
}


void ModelManager::ReloadResource(Resource* resource)
{

}


bool ModelManager::ResourceLoaded(Resource* resource, InputBlob& data)
{
	Model* model = static_cast<Model*>(resource);

	Mesh mesh;

	mesh.m_vertex_decl
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();

	int verticesCount;
	ASSERT(data.Read(verticesCount));

	size_t verticesBufferSize = verticesCount * sizeof(PosColorVertex);
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

	size_t indicesBufferSize = indicesCount * sizeof(u16);
	u16* triStrip = (u16*)m_allocator.Allocate(indicesBufferSize, ALIGN_OF(u16));
	for(int i = 0; i < indicesCount; ++i)
	{
		int num;
		ASSERT(data.Read(num));
		triStrip[i] = (u16)num;
	}

	mesh.vertexBufferHandle = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(verticesBufferSize)), mesh.m_vertex_decl);
	mesh.indexBufferHandle = bgfx::createIndexBuffer(bgfx::copy(triStrip, sizeof(indicesBufferSize)));

	m_allocator.Deallocate(vertices);
	m_allocator.Deallocate(triStrip);

	char materialPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(data.ReadLine(materialPath, Path::MAX_LENGTH));
	mesh.material = m_materialManager->Load(Path(materialPath));

	model->meshes.Push(mesh);

	return true;
}


void ModelManager::ChildResourceLoaded(resourceHandle childResource)
{
	shaderHandle childHandle = static_cast<shaderHandle>(childResource);

	for (auto& res : m_resources)
	{
		Model* model = static_cast<Model*>(res.value);
		if (model->meshes[0].material == childHandle)
		{
			FinalizeModel(model);
		}
	}
}


void ModelManager::FinalizeModel(Model* material)
{
	material->SetState(Resource::State::Ready);
}


}