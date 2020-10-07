#pragma once

#include "core/int.h"
#include "core/system.h"
#include "core/world/world.h"
#include "resource_managers/model.h"
#include "resource_managers/texture.h"
#include "resource_managers/shader.h"
#include "camera.h"
#include "light.h"
#include "core/ray.h"


namespace Veng
{

class Allocator;
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


class RenderScene : public Scene
{
public:
	struct ModelData
	{
		Path path;
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

	// model
	virtual void AddModel(Entity entity) = 0;
	virtual void RemoveModel(Entity entity) = 0;
	virtual bool HasModel(Entity entity) const = 0;
	virtual ModelData GetModelData(Entity entity) const = 0;
	virtual void SetModelData(Entity entity, const ModelData& data) = 0;

	// camera
	virtual void AddCamera(Entity entity) = 0;
	virtual void RemoveCamera(Entity entity) = 0;
	virtual bool HasCamera(Entity entity) const = 0;
	virtual const CameraItem* GetCameraData(Entity entity) const = 0;
	virtual void SetCameraData(Entity entity, const CameraItem& data) = 0;
	virtual size_t GetCamerasCount() const = 0;
	virtual const CameraItem* GetCameras() const = 0;
	virtual void SetActiveCamera(Entity entity) = 0;
	virtual const CameraItem* GetActiveCamera() const = 0;

	// directional light
	virtual void AddDirectionalLight(Entity entity) = 0;
	virtual void RemoveDirectionalLight(Entity entity) = 0;
	virtual bool HasDirectionalLight(Entity entity) const = 0;
	virtual const DirectionalLightItem* GetDirectionalLightData(Entity entity) const = 0;
	virtual void SetDirectionalLightData(Entity entity, const DirectionalLightItem& data) = 0;
	virtual size_t GetDirectionalLightsCount() const = 0;
	virtual const DirectionalLightItem* GetDirectionalLights() const = 0;

	// raycasts
	virtual bool RaycastModels(const Ray& ray, RayHit* out_hitModel = nullptr) const = 0;
};


class RenderSceneEditor : public SceneEditor
{
public:
	virtual ~RenderSceneEditor() override {}

	virtual void EditComponent(EditorInterface& editor, const ComponentBase& component, worldId world, Entity entity) override = 0;
};


class RenderSystem : public System
{
public:
	static RenderSystem* Create(Engine& engine);
	static void Destroy(RenderSystem* system);

public:
	virtual MaterialManager& GetMaterialManager() = 0;
	virtual ShaderManager& GetShaderManager() = 0;
	virtual ModelManager& GetModelManager() = 0;
	virtual TextureManager& GetTextureManager() = 0;

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
	virtual void RenderModels(World& world) = 0;
	virtual void RenderDebug() = 0;

	virtual void* GetNativeFrameBufferHandle(FramebufferHandle handle) = 0;
};


}