#pragma once

#include "core/int.h"
#include "core/iplugin.h"
#include "core/world/world.h"
#include "core/file/path.h"


namespace Veng
{

class IAllocator;
class Engine;

class RenderSystem;
class MaterialManager;
class ShaderManager;
class ModelManager;
enum class modelHandle : u64;


class RenderScene : public IScene
{
public:
	virtual ~RenderScene() override {}

	virtual void Update(float deltaTime) override = 0;

	virtual void AddModelComponent(Entity entity, worldId world, const Path& path) = 0;
	virtual void RemoveModelComponent(Entity entity, worldId world) = 0;
	virtual bool HasModelComponent(Entity entity, worldId world) const = 0;

	virtual size_t GetModelsCount() const = 0;
	virtual modelHandle* GetModels() const = 0;
};


class RenderSystem : public IPlugin
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

	virtual Engine& GetEngine() const = 0;
};


}