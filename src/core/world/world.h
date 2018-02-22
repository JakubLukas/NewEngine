#pragma once

#include "core/allocators.h"
#include "core/array.h"
#include "core/int.h"

#include "core/entity.h"
#include "core/matrix.h"


namespace Veng
{


typedef u32 worldId;


class World
{
private:
	struct EntityItem
	{
		union
		{
			Entity entity;
			i64 next;
		};
		bool alive;
	};

public:
	explicit World(IAllocator& allocator, u32 id);
	~World();

	worldId GetId() const { return m_id; }

	Entity CreateEntity();
	void DestroyEntity(Entity entity);
	Transform& GetEntityTransform(Entity entity);

private:
	IAllocator& m_allocator;
	worldId m_id;
	i64 m_unusedEntity = -1;
	Array<EntityItem> m_entities;
	Array<Transform> m_entitiesTransform;//TODO separate
};


}