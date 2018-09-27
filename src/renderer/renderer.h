#pragma once

#include "core/int.h"
#include "core/isystem.h"
#include "core/world/world.h"
#include "camera.h"////////////////////////////////


namespace Veng
{

class IAllocator;
class Engine;
struct Path;

class RenderSystem;
class MaterialManager;
class ShaderManager;
class ModelManager;
enum class modelHandle : u64;
struct Camera;


class RenderScene : public IScene
{
public:
	struct ModelItem
	{
		Entity entity;
		modelHandle handle;
	};

	struct CameraItem
	{
		Entity entity;
		Camera camera;
	};

public:
	virtual ~RenderScene() override {}

	virtual void Update(float deltaTime) override = 0;

	virtual void AddModelComponent(Entity entity, worldId world, const Path& path) = 0;
	virtual void RemoveModelComponent(Entity entity, worldId world) = 0;
	virtual bool HasModelComponent(Entity entity, worldId world) const = 0;
	virtual const ModelItem* GetModelComponent(Entity entity, worldId world) const = 0;
	virtual size_t GetModelsCount(worldId world) const = 0;
	virtual const ModelItem* GetModels(worldId world) const = 0;

	virtual void AddCameraComponent(Entity entity, worldId world, float fovY, float near, float far) = 0;
	virtual void RemoveCameraComponent(Entity entity, worldId world) = 0;
	virtual bool HasCameraComponent(Entity entity, worldId world) const = 0;
	virtual const CameraItem* GetCameraComponent(Entity entity, worldId world) const = 0;
	virtual void SetCameraFovY(Entity entity, float fovY) = 0;
	virtual void SetCameraNearFar(Entity entity, float near, float far) = 0;
	virtual void SetCameraScreenSize(Entity entity, float width, float height) = 0;
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

	virtual void Resize(u32 width, u32 height) = 0;
	virtual u32 GetScreenWidth() const = 0;
	virtual u32 GetScreenHeight() const = 0;

	virtual Engine& GetEngine() const = 0;
};


}