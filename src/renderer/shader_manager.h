#pragma once

#include <bgfx/bgfx.h>

#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"




namespace Veng
{

enum class shaderHandle : u64 {};

enum class shaderInternalHandle : u64 {};

//=============================================================================

struct ShaderInternal : public Resource
{
	bgfx::ShaderHandle handle = BGFX_INVALID_HANDLE;
};


struct ShaderProgramInternal
{
	bgfx::ProgramHandle handle = BGFX_INVALID_HANDLE;
};

//=============================================================================

struct Shader : public Resource
{
	//char* fileContent = nullptr;
	shaderInternalHandle vsHandle;
	shaderInternalHandle fsHandle;
	ShaderProgramInternal program;
	bool vsLoaded = false;
	bool fsLoaded = false;
};


//=============================================================================


class ShaderInternalManager final : public ResourceManager
{
public:
	ShaderInternalManager(IAllocator& allocator, FileSystem& fileSystem);
	~ShaderInternalManager() override;


	shaderInternalHandle Load(const Path& path);
	void Unload(shaderInternalHandle handle);
	void Reload(shaderInternalHandle handle);

	const ShaderInternal* GetResource(shaderInternalHandle handle) const;


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	bool ResourceLoaded(Resource* resource, InputBlob& data) override;
};


//=============================================================================


class ShaderManager final : public ResourceManager
{
public:
	ShaderManager(IAllocator& allocator, FileSystem& fileSystem, ShaderInternalManager* shaderInternalManager);
	~ShaderManager() override;


	shaderHandle Load(const Path& path);
	void Unload(shaderHandle handle);
	void Reload(shaderHandle handle);

	const Shader* GetResource(shaderHandle handle) const;


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	bool ResourceLoaded(Resource* resource, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle childResource) override;

	void FinalizeShader(Shader* shader);

private:
	ShaderInternalManager* m_shaderInternalManager;
};


}