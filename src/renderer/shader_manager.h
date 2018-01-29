#pragma once

#include "core/hash_map.h"

#include <bgfx/bgfx.h>




#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"
#include "core/file/file_system.h"
#include "core/file/path.h"
#include "core/array.h"
#include "core/string.h"




namespace Veng
{


/*struct ShaderInternal
{
	bgfx::ShaderHandle handle = BGFX_INVALID_HANDLE;
};

struct ShaderProgramInternal
{
	bgfx::ProgramHandle handle = BGFX_INVALID_HANDLE;
};

struct Shader
{
	ShaderInternal vertex;
	ShaderInternal fragment;
	ShaderProgramInternal program;
	//Uniforms
	//TextureSlots

	bool IsValid() const;
};


struct ProgramHashPair
{
	ShaderInternal vertex;
	ShaderInternal fragment;

	bool operator==(const ProgramHashPair& other)
	{
		return vertex.handle.idx == other.vertex.handle.idx
			&& fragment.handle.idx == other.fragment.handle.idx;
	}
	bool operator!=(const ProgramHashPair& other)
	{
		return !(operator==(other));
	}
};

template<>
struct HashCalculator<ProgramHashPair>
{
	static u32 Get(const ProgramHashPair& key)
	{
		//TODO: just dummy value to test (internally in bgfx are handles 16b values)
		return (key.vertex.handle.idx << 16u) + key.fragment.handle.idx;
	}
};


class ShaderManager final
{
public:
	ShaderManager(IAllocator& allocator);
	~ShaderManager();


	Shader GetShader(const char* vertexPath, const char* fragmentPath);

private:
	ShaderInternal* GetShaderInt(const char* path);
	ShaderProgramInternal* GetProgramInt(
		const char* vertexPath, const char* fragmentPath,
		ShaderInternal vertShaderInt, ShaderInternal fragShaderInt
	);

	IAllocator& m_allocator;
	HashMap<const char*, ShaderInternal> m_shaders;
	HashMap<ProgramHashPair, ShaderProgramInternal> m_programs;
};*/



enum shaderHandle : u64 {};

enum shaderInternalHandle : u64 {};


struct ShaderInternal : public Resource
{
	bgfx::ShaderHandle handle = BGFX_INVALID_HANDLE;
};


struct ShaderProgramInternal
{
	bgfx::ProgramHandle handle = BGFX_INVALID_HANDLE;
};

struct Shader : public Resource
{
	//char* fileContent = nullptr;
	shaderInternalHandle vsHandle;
	shaderInternalHandle fsHandle;
	ShaderProgramInternal program;
	bool vsLoaded = false;
	bool fsLoaded = false;
};



/*struct NewShaderInternal : public Resource
{
	bgfx::ShaderHandle handle = BGFX_INVALID_HANDLE;
};
*/




class ShaderInternalManager final : public ResourceManager
{
public:
	ShaderInternalManager(IAllocator& allocator, FileSystem& fileSystem);
	~ShaderInternalManager() override;


	shaderInternalHandle Load(const Path& path);
	void Unload(shaderInternalHandle handle);
	void Reload(shaderInternalHandle handle);

	ShaderInternal* GetResource(shaderInternalHandle handle);


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	bool ResourceLoaded(Resource* resource, void* const buffer, size_t bufferSize) override;
};







class ShaderManager final : public ResourceManager
{
public:
	ShaderManager(IAllocator& allocator, FileSystem& fileSystem);
	~ShaderManager() override;


	shaderHandle Load(const Path& path);
	void Unload(shaderHandle handle);
	void Reload(shaderHandle handle);

	Shader* GetResource(shaderHandle handle);


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	bool ResourceLoaded(Resource* resource, void* const buffer, size_t bufferSize) override;
	bool ChildResourceLoaded(resourceHandle childResource) override;

	bool FinalizeShader(Shader* shader);

private:
	ShaderInternalManager m_internalShaders;
};


}