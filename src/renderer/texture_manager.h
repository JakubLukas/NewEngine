#pragma once

#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"


namespace Veng
{


enum class textureHandle : u64 {};


struct Texture : public Resource
{
	u32 width = 0;
	u32 height = 0;
	u32 channels = 0;
	u8* data = nullptr;
};


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