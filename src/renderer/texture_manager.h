#pragma once

#include "core/resource/resource_manager.h"
#include "texture.h"


namespace Veng
{


class TextureManager final : public ResourceManager
{
public:
	TextureManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~TextureManager() override;


	ResourceType GetType() const override { return ResourceType::Texture; }

	textureHandle Load(const Path& path);
	void Unload(textureHandle handle);
	void Reload(textureHandle handle);

	const Texture* GetResource(textureHandle handle) const;


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;
};


}