#pragma once

#include "core/array.h"

#include <bgfx/bgfx.h>


namespace Veng
{

struct ShaderInternal
{
	bgfx::ShaderHandle handle;
};

struct ShaderProgramInternal
{
	bgfx::ProgramHandle handle;
};

struct Shader
{
	ShaderInternal vertex;
	ShaderInternal fragment;
	ShaderProgramInternal program;
	//Uniforms
	//TextureSlots
};

//TODO move ?
struct Material
{
	Shader* shader;
	//Textures
	//Uniforms?
	//commandBuffer*
};


struct Mesh
{
	bgfx::VertexDecl m_vertex_decl;
	bgfx::VertexBufferHandle vertexBufferHandle;
	bgfx::IndexBufferHandle indexBufferHandle;
	Material* material;
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