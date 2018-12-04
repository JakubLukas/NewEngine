#pragma once

#include "core/containers/array.h"
#include "core/resource/resource.h"
#include "material_manager.h"

#include <bgfx/bgfx.h>


namespace Veng
{


struct Shader;





struct Mesh
{
	bgfx::VertexDecl m_vertex_decl;
	bgfx::VertexBufferHandle vertexBufferHandle;
	bgfx::IndexBufferHandle indexBufferHandle;
	materialHandle material;
};


struct Model : public Resource
{
	Model(IAllocator& allocator)
		: Resource(ResourceType::Model)
		, meshes(allocator)
	{}
	~Model()
	{}

	Array<Mesh> meshes;
	//LODs
	//Bones
	//Skins
	//AABB
	//boundSphereRadius
};


}