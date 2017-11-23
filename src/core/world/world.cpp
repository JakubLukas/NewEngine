#include "world.h"

#include "scene.h"


namespace Veng
{


World::World(IAllocator& allocator, u32 id)
	: m_allocator(allocator)
	, m_entities(m_allocator)
	, m_id(id)
{

}


World::~World()
{

}


Entity World::CreateEntity()
{
	if (m_aliveEntitiesCount < m_entities.GetSize())
	{
		++m_aliveEntitiesCount;
		return m_entities[m_aliveEntitiesCount - 1];
	}
	else
	{
		++m_currentEntityId;
		m_entities.Push((Entity)m_currentEntityId);
		return (Entity)m_currentEntityId;
	}
}


void World::DestroyEntity(Entity entity)
{
	for (unsigned i = 0; i < m_entities.GetSize(); ++i)
	{
		if (m_entities[i] == entity)
		{
			--m_aliveEntitiesCount;
			m_entities[i] = m_entities[m_aliveEntitiesCount];
			return;
		}
	}
	ASSERT2(false, "DestroyEntity: entity to destroy doesn't exist");
}


}