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
				textures[i] = (textureHandle)INVALID_HANDLE;
		}

		materialHandle material;
		shaderHandle shader;
		textureHandle textures[Material::MAX_TEXTURES];
		u8 shaderLoaded = 0;
		u8 texturesLoaded = 0;
	};

public:
	MaterialManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~MaterialManager() override;


	ResourceType GetType() const override { return ResourceType::Material; };

	materialHandle Load(const Path& path);
	void Unload(materialHandle handle);
	void Reload(materialHandle handle);

	const Material* GetResource(materialHandle handle) const;

	void SetRenderSystem(RenderSystem* renderSystem);

private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle handle, ResourceType type) override;

	void FinalizeMaterial(Material* material);

private:
	RenderSystem* m_renderSystem;
	Array<LoadingOp> m_loadingOp;
};


}