#pragma once

#include "core/iplugin.h"


namespace Veng
{

class IAllocator;
class Engine;
typedef u32 worldId;


struct Mesh
{

};


class RenderSystem : public IPlugin
{
public:
	static RenderSystem* Create(Engine& engine);
	static void Destroy(RenderSystem* system);


	virtual ~RenderSystem() override {}

	virtual void Update(float deltaTime) override = 0;
	virtual const char* GetName() const override = 0;


	virtual void AddMeshComponent(Entity entity, worldId world) = 0;
	virtual void RemoveMeshComponent(Entity entity, worldId world) = 0;
	virtual bool HasMeshComponent(Entity entity, worldId world) = 0;

	virtual Engine& GetEngine() const = 0;
};


}