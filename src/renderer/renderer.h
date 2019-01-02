#pragma once

#include "core/int.h"
#include "core/isystem.h"
#include "core/world/world.h"
#include "resource_managers/model.h"
#include "resource_managers/texture.h"
#include "resource_managers/shader.h"
#include "camera.h"


namespace Veng
{

class IAllocator;
class Engine;
class Path;
class InputBlob;

class RenderSystem;
class MaterialManager;
class ShaderManager;
class ModelManager;
class TextureManager;


class RenderScene : public IScene
{
public:
	struct ModelItem
	{
		Entity entity;
		resourceHandle model;
	};

	struct CameraItem
	{
		Entity entity;
		Camera camera;
	};

public:
	virtual ~RenderScene() override {}

	virtual void Update(float deltaTime) override = 0;

	virtual size_t GetComponentCount() const override = 0;
	virtual const ComponentInfo* GetComponents() const override = 0;
	virtual const ComponentInfo* GetComponentInfo(componentHandle handle) const override = 0;

	virtual void AddComponent(componentHandle handle, Entity entity, worldId world) override = 0;
	virtual void RemoveComponent(componentHandle handle, Entity entity, worldId world) override = 0;
	virtual bool HasComponent(componentHandle handle, Entity entity, worldId world) const override = 0;
	virtual void GetComponentData(componentHandle handle, Entity entity, worldId world, void* buffer) const override = 0;
	virtual void SetComponentData(componentHandle handle, Entity entity, worldId world, void* data) override = 0;

	virtual size_t GetModelsCount(worldId world) const = 0;
	virtual const ModelItem* GetModels(worldId world) const = 0;

	virtual size_t GetCamerasCount(worldId world) const = 0;
	virtual const CameraItem* GetCameras(worldId world) const = 0;
	virtual const CameraItem* GetDefaultCamera(worldId world) = 0;
};


class RenderSystem : public ISystem
{
public:
	static RenderSystem* Create(Engine& engine);
	static void Destroy(RenderSystem* system);

public:
	virtual ~RenderSystem() override {}

	virtual void Init() override = 0;

	virtual void Update(float deltaTime) override = 0;
	virtual const char* GetName() const override = 0;

	virtual IScene* GetScene() const override = 0;

	virtual MaterialManager& GetMaterialManager() const = 0;
	virtual ShaderManager& GetShaderManager() const = 0;
	virtual ModelManager& GetModelManager() const = 0;
	virtual TextureManager& GetTextureManager() const = 0;

	virtual meshRenderHandle CreateMeshData(InputBlob& data) = 0;
	virtual void DestroyMeshData(meshRenderHandle handle) = 0;

	virtual textureRenderHandle CreateTextureData(const Texture& texture) = 0;
	virtual void DestroyTextureData(textureRenderHandle handle) = 0;

	virtual shaderInternalRenderHandle CreateShaderInternalData(InputBlob& data) = 0;
	virtual void DestroyShaderInternalData(shaderInternalRenderHandle handle) = 0;

	virtual shaderRenderHandle CreateShaderData(shaderInternalRenderHandle vsHandle, shaderInternalRenderHandle fsHandle) = 0;
	virtual void DestroyShaderData(shaderRenderHandle handle) = 0;

	virtual void Resize(u32 width, u32 height) = 0;
	virtual u32 GetScreenWidth() const = 0;
	virtual u32 GetScreenHeight() const = 0;

	virtual Engine& GetEngine() const = 0;
};


}