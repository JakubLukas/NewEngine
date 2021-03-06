#pragma once

#include "core/allocator.h"
#include "core/containers/array.h"
#include "core/int.h"

#include "core/entity.h"


namespace Veng
{

struct Transform;
enum class worldId : u32 {};
static const worldId INVALID_WORLD_ID = (worldId)-1;


class World
{
public:
	class EntityIterator
	{
	public:
		EntityIterator(const World& world)
			: m_world(world)
		{}

		bool GetNext(Entity& outEntity)
		{
			for (m_index; m_index < m_world.m_entities.GetSize(); ++m_index)
			{
				if (m_world.m_entities[m_index].alive)
				{
					outEntity = m_world.m_entities[m_index].entity;
					m_index++;
					return true;
				}
			}
			return false;
		}

	private:
		size_t m_index = 0;
		const World& m_world;
	};

public:
	World(Allocator& allocator, worldId id);
	World(World&& world);
	~World();

	worldId GetId() const { return m_id; }

	void Serialize(class OutputBlob& serializer) const;
	void Deserialize(class InputBlob& serializer);

	Entity CreateEntity();
	void DestroyEntity(Entity entity);
	bool ExistsEntity(Entity entity);
	Transform& GetEntityTransform(Entity entity);
	EntityIterator GetEntities() const;

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

private:
	Allocator& m_allocator;
	worldId m_id;
	i64 m_unusedEntity = -1;
	Array<EntityItem> m_entities;
	Array<Transform> m_entitiesTransform;//TODO separate
};


}