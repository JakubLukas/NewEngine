#pragma once

#include "core/resource/resource_manager.h"
#include "model.h"


namespace Veng
{

class RenderSystem;


class ModelManager final : public ResourceManager
{
public:
	ModelManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~ModelManager() override;


	ResourceType GetType() const override { return ResourceType::Model; };

	modelHandle Load(const Path& path);
	void Unload(modelHandle handle);
	void Reload(modelHandle handle);

	const Model* GetResource(modelHandle handle) const;

	void SetRenderSystem(RenderSystem* renderSystem);

private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle handle, ResourceType type) override;

	void FinalizeModel(Model* model);

private:
	RenderSystem* m_renderSystem;
};


}