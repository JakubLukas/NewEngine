#pragma once

#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"

#include "shader_manager.h"


namespace Veng
{

enum materialHandle : u64 {};


struct Material : public Resource
{
	shaderHandle shader;
	//Textures
	//Uniforms?
	//commandBuffer*
};


class MaterialManager final : public ResourceManager
{
public:
	MaterialManager(IAllocator& allocator, FileSystem& fileSystem);
	~MaterialManager() override;


	materialHandle Load(const Path& path);
	void Unload(materialHandle handle);
	void Reload(materialHandle handle);

	const Material* GetResource(materialHandle handle) const;


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	bool ResourceLoaded(Resource* resource, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle childResource) override;

	void FinalizeMaterial(Material* shader);

private:
	//ShaderInternalManager m_internalShaders;
};


}