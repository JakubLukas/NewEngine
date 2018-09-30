#pragma once

#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"


namespace Veng
{


enum class textureHandle : u64 {};


struct Texture : public Resource
{
	//format
	u8* data;
};


class TextureManager final : public ResourceManager
{
public:
	TextureManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~TextureManager() override;


	textureHandle Load(const Path& path);
	void Unload(textureHandle handle);
	void Reload(textureHandle handle);

	const Texture* GetResource(textureHandle handle) const;


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputClob& data) override;
	void ChildResourceLoaded(resourceHandle childResource) override;

	void FinalizeTexture(Texture* material);
};


}