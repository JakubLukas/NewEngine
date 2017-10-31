#include "entity_manager.h"

#include "core/array.h"


namespace Veng
{


class EntityManagerImpl : public EntityManager
{
public:
	EntityManagerImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_entities(m_allocator)
	{

	}

	~EntityManagerImpl()
	{

	}

	Entity CreateEntity() override
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

	void DestroyEntity(Entity entity) override
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

private:
	IAllocator& m_allocator;
	unsigned m_currentId = 0;
	unsigned m_aliveEntitiesCount = 0;
	Array<Entity> m_entities;
	//notify other systems on create / destroy
	//IsAlive(Entity) ??? if so, speedup
};


EntityManager* EntityManager::Create(IAllocator& allocator)
{
	return new (allocator, ALIGN_OF(EntityManagerImpl)) EntityManagerImpl(allocator);
}


void EntityManager::Destroy(EntityManager* inst, IAllocator& allocator)
{
	auto* p = (EntityManagerImpl*)inst;
	p->~EntityManagerImpl();
	allocator.Deallocate(p);
}


}