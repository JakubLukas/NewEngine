#pragma once

#include "core/array.h"
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
	Material* material;

	bool Load(/*FileBlob*/);
	void Clear();
};


struct Model
{
	Model(IAllocator& allocator);
	~Model();

	Array<Mesh> meshes;
	//LODs
	//Bones
	//Skins
	//AABB
	//boundSphereRadius
};


}