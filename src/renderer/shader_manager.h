#pragma once

#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"

#include <../external/bgfx/include/bgfx/bgfx.h>//should be just <bgfx/bgfx.h> but this header is used in script system, and that doesn't include ../../external/bgfx/include


namespace Veng
{

enum class shaderHandle : u64 {};

enum class shaderInternalHandle : u64 {};

//=============================================================================

struct ShaderInternal : public Resource
{
	ShaderInternal() : Resource(ResourceType::ShaderInternal) {}

	bgfx::ShaderHandle handle = BGFX_INVALID_HANDLE;
};


struct ShaderProgramInternal
{
	bgfx::ProgramHandle handle = BGFX_INVALID_HANDLE;
};

//=============================================================================

struct Shader : public Resource
{
	Shader() : Resource(ResourceType::Shader) {}

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
	ShaderInternalManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~ShaderInternalManager() override;


	ResourceType GetType() const override { return ResourceType::ShaderInternal; };

	shaderInternalHandle Load(const Path& path);

	void Unload(shaderInternalHandle handle);
	void Reload(shaderInternalHandle handle);

	const ShaderInternal* GetResource(shaderInternalHandle handle) const;


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;
};


//=============================================================================


class ShaderManager final : public ResourceManager
{
public:
	ShaderManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~ShaderManager() override;


	ResourceType GetType() const override { return ResourceType::Shader; };

	shaderHandle Load(const Path& path);
	void Unload(shaderHandle handle);
	void Reload(shaderHandle handle);

	const Shader* GetResource(shaderHandle handle) const;


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle handle, ResourceType type) override;

	void FinalizeShader(Shader* shader);
};


}