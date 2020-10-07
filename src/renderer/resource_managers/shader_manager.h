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
	ShaderInternalManager(Allocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~ShaderInternalManager() override;

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
public:
	ShaderManager(Allocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~ShaderManager() override;

	const char* const * GetSupportedFileExt() const override;
	size_t GetSupportedFileExtCount() const override;

	void SetRenderSystem(RenderSystem* renderSystem);

private:
	struct LoadingOp
	{
		enum ShaderBits : u8
		{
			Shader_None = 0,
			Shader_Vertex = 1 << 0,
			Shader_Fragment = 1 << 1,
		};
		resourceHandle shader;
		resourceHandle vsHandle;
		resourceHandle fsHandle;
		u8 shadersToLoad = Shader_None;
	};

private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle handle, ResourceType type) override;

	void FinalizeShader(Shader* shader);

	static bool LoadingOpCompleted(const LoadingOp& op);

private:
	RenderSystem* m_renderSystem;
	Array<LoadingOp> m_loadingOp;
};


}