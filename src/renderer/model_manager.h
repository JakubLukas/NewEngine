#pragma once

#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"

#include "model.h"
#include "material_manager.h"


namespace Veng
{


enum modelHandle : u64 {};


class ModelManager final : public ResourceManager
{
public:
	ModelManager(IAllocator& allocator, FileSystem& fileSystem, MaterialManager* materialManager);
	~ModelManager() override;


	modelHandle Load(const Path& path);
	void Unload(modelHandle handle);
	void Reload(modelHandle handle);

	const Model* GetResource(modelHandle handle) const;


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	bool ResourceLoaded(Resource* resource, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle childResource) override;

	void FinalizeModel(Model* material);

private:
	MaterialManager* m_materialManager;
};


}