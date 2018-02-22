#include "world.h"

#include "scene.h"


namespace Veng
{


World::World(IAllocator& allocator, u32 id)
	: m_allocator(allocator)
	, m_entities(m_allocator)
	, m_id(id)
	, m_entitiesTransform(m_allocator)
{

}


World::~World()
{

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
		EntityItem item = m_entities.Push();
		item.entity = (Entity)m_entities.GetSize();
		item.alive = true;
		m_entitiesTransform.Push();
		return item.entity;
	}
}


void World::DestroyEntity(Entity entity)
{
	size_t id = (size_t)entity;
	ASSERT2(id < m_entities.GetSize(), "DestroyEntity: entity to destroy doesn't exist");

	ASSERT(m_entities[id].alive == false);

	m_entities[id].alive = false;
	m_entities[id].next = m_unusedEntity;
	m_unusedEntity = (i64)id;
}

Transform& World::GetEntityTransform(Entity entity)
{
	size_t id = (size_t)entity;
	ASSERT(id < m_entities.GetSize())
	return m_entitiesTransform[id];
}


}