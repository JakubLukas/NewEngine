#pragma once

#include "core/int.h"
#include "core/isystem.h"
#include "core/world/world.h"
#include "resource_managers/model.h"
#include "resource_managers/texture.h"
#include "resource_managers/shader.h"
#include "camera.h"
#include "light.h"
#include "core/ray.h"


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


enum class FramebufferHandle : u16 {};
const FramebufferHandle INVALID_FRAMEBUFFER_HANDLE = (FramebufferHandle)0xffff;

typedef u8 FramebufferTypeFlags;
enum FramebufferTypeBits : FramebufferTypeFlags
{
	FramebufferType_None = 0,
	FramebufferType_Color = 1 << 0,//rgb
	FramebufferType_Depth = 1 << 1,//32b float
};


class RenderScene : public IScene
{
public:
	enum class Component : u8
	{
		Model = 0,
		Camera = 1,
		DirectionalLight = 2,

		Count
	};

	static componentHandle GetComponentHandle(Component comp);

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

	struct DirectionalLightItem
	{
		Entity entity;
		struct DirectionalLight light;
	};

public:
	virtual ~RenderScene() override {}

	virtual void Update(float deltaTime) override = 0;

	virtual size_t GetComponentCount() const override = 0;
	virtual const ComponentInfo* GetComponentInfos() const override = 0;
	virtual const ComponentInfo* GetComponentInfo(componentHandle handle) const override = 0;
	virtual componentHandle GetComponentHandle(const char* name) const override = 0;

	virtual void AddComponent(componentHandle handle, Entity entity) override = 0;
	virtual void RemoveComponent(componentHandle handle, Entity entity) override = 0;
	virtual bool HasComponent(componentHandle handle, Entity entity) const override = 0;
	virtual void EditComponent(EditorInterface* editor, componentHandle handle, Entity entity) override = 0;
	virtual void* GetComponentData(componentHandle handle, Entity entity) const override = 0;
	virtual void SetComponentData(componentHandle handle, Entity entity, void* data) override = 0;

	virtual size_t GetModelsCount() const = 0;
	virtual const ModelItem* GetModels() const = 0;

	virtual size_t GetCamerasCount() const = 0;
	virtual const CameraItem* GetCameras() const = 0;
	virtual void SetMainCamera(Entity entity) = 0;
	virtual const CameraItem* GetMainCamera() const = 0;

	virtual size_t GetDirectionalLightsCount() const = 0;
	virtual const DirectionalLightItem* GetDirectionalLights() const = 0;

	virtual bool RaycastModels(const Ray& ray, const ModelItem* hitModel) const = 0;
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

	virtual IScene* GetScene(worldId world) const override = 0;

	virtual void OnWorldAdded(worldId world) override = 0;
	virtual void OnWorldRemoved(worldId world) override = 0;

	virtual MaterialManager& GetMaterialManager() const = 0;
	virtual ShaderManager& GetShaderManager() const = 0;
	virtual ModelManager& GetModelManager() const = 0;
	virtual TextureManager& GetTextureManager() const = 0;

	virtual meshRenderHandle CreateMeshData(Mesh& mesh) = 0;
	virtual void DestroyMeshData(meshRenderHandle handle) = 0;

	virtual materialRenderHandle CreateMaterialData(Material& material) = 0;
	virtual void DestroyMaterialData(materialRenderHandle handle) = 0;

	virtual textureRenderHandle CreateTextureData(const Texture& texture) = 0;
	virtual void DestroyTextureData(textureRenderHandle handle) = 0;

	virtual shaderInternalRenderHandle CreateShaderInternalData(InputBlob& data) = 0;
	virtual void DestroyShaderInternalData(shaderInternalRenderHandle handle) = 0;

	virtual shaderRenderHandle CreateShaderData(shaderInternalRenderHandle vsHandle, shaderInternalRenderHandle fsHandle) = 0;
	virtual void DestroyShaderData(shaderRenderHandle handle) = 0;

	virtual void AddDebugLine(Vector3 from, Vector3 to, Color color, float width, float lifetime) = 0;
	virtual void AddDebugSquare(Vector3 position, Color color, float size, float lifetime) = 0;

	virtual void Resize(u32 width, u32 height) = 0;
	virtual u32 GetScreenWidth() const = 0;
	virtual u32 GetScreenHeight() const = 0;

	virtual Engine& GetEngine() const = 0;

	//render api
	virtual FramebufferHandle CreateFrameBuffer(int width, int height, bool screenSize, FramebufferTypeFlags flags) = 0;
	virtual void DestroyFramebuffer(FramebufferHandle handle) = 0;

	virtual void NewView() = 0;
	virtual void SetFramebuffer(FramebufferHandle handle) = 0;
	virtual void SetCamera(World& world, Entity camera) = 0;
	virtual void Clear() = 0;
	virtual void RenderModels(World& world, const RenderScene::ModelItem* models, size_t count) = 0;
	virtual void RenderDebug() = 0;

	virtual void* GetNativeFrameBufferHandle(FramebufferHandle handle) = 0;
};


}