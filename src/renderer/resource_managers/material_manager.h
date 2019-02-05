#pragma once

#include "core/resource/resource_manager.h"
#include "material.h"

#include "core/containers/array.h"


namespace Veng
{

class RenderSystem;


class MaterialManager final : public ResourceManager
{
private:
	struct LoadingOp
	{
		LoadingOp()
		{
			for (size_t i = 0; i < Material::MAX_TEXTURES; ++i)
				textures[i] = INVALID_RESOURCE_HANDLE;
		}

		resourceHandle material;
		resourceHandle shader;
		resourceHandle textures[Material::MAX_TEXTURES];
		u8 shaderLoaded = 0;
		u8 texturesToLoad = 0;
	};

public:
	MaterialManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~MaterialManager() override;

	ResourceType GetType() const override { return ResourceType::Material; };

	const char* const * GetSupportedFileExt() const override;
	size_t GetSupportedFileExtCount() const override;

	void SetRenderSystem(RenderSystem* renderSystem);

private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle handle, ResourceType type) override;

	void FinalizeMaterial(Material* material);

	static bool LoadingOpCompleted(const LoadingOp& op);

private:
	RenderSystem* m_renderSystem;
	Array<LoadingOp> m_loadingOp;
};


}