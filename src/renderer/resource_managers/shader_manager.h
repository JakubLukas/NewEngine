#pragma once

#include "core/resource/resource_manager.h"
#include "shader.h"

#include "core/containers/array.h"


namespace Veng
{

class RenderSystem;


class ShaderInternalManager final : public ResourceManager
{
public:
	ShaderInternalManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~ShaderInternalManager() override;


	ResourceType GetType() const override { return ResourceType::ShaderInternal; };

	const char* const * GetSupportedFileExt() const override;
	size_t GetSupportedFileExtCount() const override;

	void SetRenderSystem(RenderSystem* renderSystem);

private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;

private:
	RenderSystem* m_renderSystem;
};


//=============================================================================


class ShaderManager final : public ResourceManager
{
private:
	struct LoadingOp
	{
		resourceHandle shader;
		resourceHandle vsHandle;
		resourceHandle fsHandle;
		u8 shadersIntLoaded = 0;
	};

public:
	ShaderManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~ShaderManager() override;


	ResourceType GetType() const override { return ResourceType::Shader; };

	const char* const * GetSupportedFileExt() const override;
	size_t GetSupportedFileExtCount() const override;

	void SetRenderSystem(RenderSystem* renderSystem);

private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle handle, ResourceType type) override;

	void FinalizeShader(Shader* shader);

private:
	RenderSystem* m_renderSystem;
	Array<LoadingOp> m_loadingOp;
};


}