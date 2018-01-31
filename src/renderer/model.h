#pragma once

#include "core/array.h"
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

	bool Load(/*FileBlob*/);
	void Clear();
};


struct Model : public Resource
{
	Model(IAllocator& allocator)
		: meshes(allocator)
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