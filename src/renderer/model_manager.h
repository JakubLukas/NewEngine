#pragma once

#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"

#include "model.h"
#include "material_manager.h"


namespace Veng
{


enum class modelHandle : u64 {};


class ModelManager final : public ResourceManager
{
public:
	ModelManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~ModelManager() override;


	modelHandle Load(const Path& path);
	void Unload(modelHandle handle);
	void Reload(modelHandle handle);

	const Model* GetResource(modelHandle handle) const;


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle childResource) override;

	void FinalizeModel(Model* model);
};


}