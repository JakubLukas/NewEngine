#pragma once

#include "core/resource/resource.h"
#include "core/resource/resource_manager.h"

#include "shader_manager.h"
#include "texture_manager.h"

#include "core/array.h"


namespace Veng
{


enum class materialHandle : u64 {};


struct Material : public Resource
{
	shaderHandle shader;
	static const int MAX_TEXTURES = 4;
	textureHandle textures[MAX_TEXTURES] = { (textureHandle)INVALID_HANDLE };
	//Uniforms?
	//commandBuffer*
};


class MaterialManager final : public ResourceManager
{
private:
	struct MaterialLoadingOp
	{
		bool shader = false;
		bool textures[Material::MAX_TEXTURES] = { 0 };
	};

public:
	MaterialManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~MaterialManager() override;


	materialHandle Load(const Path& path);
	void Unload(materialHandle handle);
	void Reload(materialHandle handle);

	const Material* GetResource(materialHandle handle) const;


private:
	Resource* CreateResource() override;
	void DestroyResource(Resource* resource) override;
	void ReloadResource(Resource* resource) override;
	void ResourceLoaded(resourceHandle handle, InputBlob& data) override;
	void ChildResourceLoaded(resourceHandle handle, ResourceType type) override;

	void FinalizeMaterial(Material* material);

private:
	Array<MaterialLoadingOp> m_loadingOp;
};


}