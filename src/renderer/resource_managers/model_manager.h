#pragma once

#include "core/resource/resource_manager.h"
#include "model.h"


namespace Veng
{

class RenderSystem;


class ModelManager final : public ResourceManager
{
public:
	ModelManager(Allocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~ModelManager() override;

	const char* const * GetSupportedFileExt() const override;
	size_t GetSupportedFileExtCount() const override;

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