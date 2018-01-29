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
	bool ResourceLoaded(Resource* resource, InputBlob& data) override;
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
	bool ResourceLoaded(Resource* resource, InputBlob& data) override;
	bool ChildResourceLoaded(resourceHandle childResource) override;

	bool FinalizeShader(Shader* shader);

private:
	ShaderInternalManager m_internalShaders;
};


}