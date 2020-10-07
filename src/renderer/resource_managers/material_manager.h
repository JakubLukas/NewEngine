#pragma once

#include "core/resource/resource_manager.h"
#include "material.h"

#include "core/containers/array.h"


namespace Veng
{

class RenderSystem;


class MaterialManager final : public ResourceManager
{
public:
	MaterialManager(Allocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~MaterialManager() override;

	const char* const * GetSupportedFileExt() const override;
	size_t GetSupportedFileExtCount() const override;

	void SetRenderSystem(RenderSystem* renderSystem);

private:
	struct LoadingOp
	{
		resourceHandle material;
		resourceHandle shader;
		resourceHandle textures[Material::MAX_TEXTURES];
		u8 textureCount = 0;
		u8 shaderLoaded = 0;
		u8 texturesToLoad = 0;//TODO: ^ merge to one bitflag field
	};

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