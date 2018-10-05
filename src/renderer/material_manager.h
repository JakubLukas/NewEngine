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
		materialHandle material;
		shaderHandle shader;
		bool shaderLoaded = false;
		textureHandle textures[Material::MAX_TEXTURES] = { (textureHandle)INVALID_HANDLE };
		bool texturesLoaded[Material::MAX_TEXTURES] = { false };//use bitfields
	};

public:
	MaterialManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager);
	~MaterialManager() override;


	ResourceType GetType() const override { return ResourceType::Material; };

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