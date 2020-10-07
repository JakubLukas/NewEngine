#include "world.h"

#include "scene.h"
#include "core/math/matrix.h"
#include "core/utility.h"
#include "core/file/blob.h"


namespace Veng
{


World::World(Allocator& allocator, worldId id)
	: m_allocator(allocator)
	, m_entities(m_allocator)
	, m_id(id)
	, m_entitiesTransform(m_allocator)
{

}


World::World(World&& world)
	: m_allocator(world.m_allocator)
	, m_entities(Utils::Move(world.m_entities))
	, m_id(world.m_id)
	, m_entitiesTransform(Utils::Move(world.m_entitiesTransform))
{

}


World::~World()
{

}


void World::Serialize(OutputBlob& serializer) const
{
	serializer.Write(m_unusedEntity);

	serializer.Write((u64)m_entities.GetSize());
	serializer.Write(m_entities.Begin(), sizeof(EntityItem) * m_entities.GetSize());

	serializer.Write((u64)m_entitiesTransform.GetSize());
	serializer.Write(m_entitiesTransform.Begin(), sizeof(Transform) * m_entitiesTransform.GetSize());
}

void World::Deserialize(InputBlob& serializer)
{
	serializer.Read(m_unusedEntity);

	u64 count;
	serializer.Read(count);
	m_entities.Resize(count);
	serializer.Read(m_entities.Begin(), sizeof(EntityItem) * count);

	serializer.Read(count);
	m_entitiesTransform.Resize(count);
	serializer.Read(m_entitiesTransform.Begin(), sizeof(Transform) * count);
}


Entity World::CreateEntity()
{
	if (m_unusedEntity != -1)
	{
		size_t id = (size_t)m_unusedEntity;
		m_unusedEntity = m_entities[id].next;
		m_entities[id].entity = (Entity)id;
		m_entities[id].alive = true;
		return m_entities[id].entity;
	}
	else
	{
		EntityItem& item = m_entities.PushBack();
		item.entity = (Entity)(m_entities.GetSize() - 1);
		item.alive = true;
		m_entitiesTransform.PushBack();
		return item.entity;
	}
}


void World::DestroyEntity(Entity entity)
{
	size_t id = (size_t)entity;
	ASSERT2(id < m_entities.GetSize(), "Invalid entity");

	ASSERT2(m_entities[id].alive, "Entity has been already destroyed");

	m_entities[id].alive = false;
	m_entities[id].next = m_unusedEntity;
	m_unusedEntity = (i64)id;
}

bool World::ExistsEntity(Entity entity)
{
	size_t idx = (size_t)entity;
	ASSERT2(idx < m_entities.GetSize(), "Invalid entity");
	return m_entities[idx].alive;
}

Transform& World::GetEntityTransform(Entity entity)
{
	size_t idx = (size_t)entity;
	ASSERT2(idx < m_entities.GetSize(), "Invalid entity");
	ASSERT2(m_entities[idx].alive, "Entity is destroyed");
	return m_entitiesTransform[idx];
}


World::EntityIterator World::GetEntities() const
{
	return World::EntityIterator(*this);
}


}