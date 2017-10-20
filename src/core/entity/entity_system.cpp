#include "entity_system.h"

#include "core/array.h"


namespace Veng
{


class EntitySystemImpl : public EntitySystem
{
public:
	EntitySystemImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_entities(m_allocator)
	{

	}

	~EntitySystemImpl()
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
			m_entities.Push(m_currentId);
			return m_currentId;
		}
	}

	void DestroyEntity(Entity entity) override
	{
		unsigned index;
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


EntitySystem* EntitySystem::Create(IAllocator& allocator)
{
	void* p = allocator.Allocate(sizeof(EntitySystemImpl));
	return new (NewPlaceholder(), p) EntitySystemImpl(allocator);
}


void EntitySystem::Destroy(EntitySystem* system, IAllocator& allocator)
{
	EntitySystemImpl* p = (EntitySystemImpl*)system;
	p->~EntitySystemImpl();
	allocator.Deallocate(p);
}


}