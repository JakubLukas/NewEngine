#pragma once

#include "core/resource/resource_manager.h"
#include "texture.h"


namespace Veng
{

class RenderSystem;


class TextureManager final : public ResourceManager
{
public:
	TextureManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~TextureManager() override;


	ResourceType GetType() const override { return ResourceType::Texture; }

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


}