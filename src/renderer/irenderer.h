#pragma once

#include "core/iplugin.h"
#include "core/file/path.h"


namespace Veng
{

class IAllocator;
class Engine;
typedef u32 worldId;


class RenderSystem : public IPlugin
{
public:
	static RenderSystem* Create(Engine& engine);
	static void Destroy(RenderSystem* system);

public:
	virtual ~RenderSystem() override {}

	virtual void Update(float deltaTime) override = 0;
	virtual const char* GetName() const override = 0;

	virtual void Resize(u32 width, u32 height) = 0;


	virtual void AddModelComponent(Entity entity, worldId world, const Path& path) = 0;
	virtual void RemoveModelComponent(Entity entity, worldId world) = 0;
	virtual bool HasModelComponent(Entity entity, worldId world) = 0;

	virtual Engine& GetEngine() const = 0;
};


}