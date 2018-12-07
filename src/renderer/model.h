#pragma once

#include "core/resource/resource.h"
#include "core/containers/array.h"

#include "material.h"


namespace Veng
{

enum class modelHandle : u64 {};
const modelHandle INVALID_MODEL_HANDLE = (modelHandle)0;

enum class meshRenderHandle : u64 {};
const meshRenderHandle INVALID_MODEL_RENDER_HANDLE = (meshRenderHandle)0;


struct Mesh
{
	materialHandle material;
	meshRenderHandle renderDataHandle;
};


struct Model : public Resource
{
	Model(IAllocator& allocator)
		: Resource(ResourceType::Model)
		, meshes(allocator)
	{}

	Array<Mesh> meshes;
	//LODs
	//Bones
	//Skins
	//AABB
	//boundSphereRadius
};


}