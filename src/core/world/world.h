#pragma once

#include "core/allocators.h"
#include "core/array.h"
#include "core/int.h"


namespace Veng
{


enum class Entity : u32;
class Scene;

typedef u32 worldId;

class World
{
public:
	explicit World(IAllocator& allocator, u32 id);
	~World();

	worldId GetId() const { return m_id; }

	Entity CreateEntity();
	void DestroyEntity(Entity entity);

private:
	IAllocator& m_allocator;
	worldId m_id;
	unsigned m_currentEntityId = 0;
	unsigned m_aliveEntitiesCount = 0;
	Array<Entity> m_entities;
	//Array<Transform> m_entitiesTransform;
};


}