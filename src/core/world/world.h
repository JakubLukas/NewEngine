#pragma once

#include "core/allocators.h"
#include "core/array.h"
#include "core/int.h"


namespace Veng
{


enum class Entity : u32;
class Scene;


class World
{
public:
	explicit World(IAllocator& allocator);
	~World();

	Entity CreateEntity();
	void DestroyEntity(Entity entity);

private:
	IAllocator& m_allocator;
	unsigned m_currentId = 0;
	unsigned m_aliveEntitiesCount = 0;
	Array<Entity> m_entities;
	//Array<Transform> m_entitiesTransform;
};


}