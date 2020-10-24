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
	enum class PrimitiveType : u8 { Points = 1, Lines = 2, Triangles = 3 };

	PrimitiveType type;
	//24b padding
	resourceHandle material = INVALID_RESOURCE_HANDLE;
	ShaderVaryingFlags varyings = ShaderVarying_None;

	meshRenderHandle renderDataHandle = INVALID_MESH_RENDER_HANDLE;

	u8* verticesData = nullptr;
	u16* indicesData = nullptr;
	u32 verticesCount = 0;
	u32 indicesCount = 0;
};


struct Model : public Resource
{
	static ResourceType RESOURCE_TYPE;

	Model(Allocator& allocator)
		: Resource(ResourceType("model"))
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