#include "world.h"

#include "core/entity/entity_manager.h"
#include "scene.h"


namespace Veng
{


World::World(IAllocator& allocator)
	: m_allocator(allocator)
	, m_entities(m_allocator)
{

}


World::~World()
{

}


Entity World::CreateEntity()
{
	if (m_aliveEntitiesCount < m_entities.Size())
	{
		++m_aliveEntitiesCount;
		return m_entities[m_aliveEntitiesCount - 1];
	}
	else
	{
		++m_currentId;
		m_entities.Push((Entity)m_currentId);
		return (Entity)m_currentId;
	}
}


void World::DestroyEntity(Entity entity)
{
	for (unsigned i = 0; i < m_entities.Size(); ++i)
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