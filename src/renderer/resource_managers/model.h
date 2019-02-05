#pragma once

#include "core/resource/resource.h"
#include "core/containers/array.h"

#include "material.h"


namespace Veng
{

enum class meshRenderHandle : u32 {};
const meshRenderHandle INVALID_MESH_RENDER_HANDLE = (meshRenderHandle)0;


struct Mesh
{
	resourceHandle material = INVALID_RESOURCE_HANDLE;
	ShaderVaryingFlags varyings = SV_NONE;
	//32b padding
	u8* verticesData = nullptr;
	u16* indicesData = nullptr;
	u32 verticesCount = 0;
	u32 indicesCount = 0;
	meshRenderHandle renderDataHandle = INVALID_MESH_RENDER_HANDLE;
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