#pragma once

#include "core/allocators.h"
#include "core/int.h"


namespace Veng
{


enum class Entity : u32 {};


class EntityManager
{
public:
	static EntityManager* Create(IAllocator& allocator);
	static void Destroy(EntityManager* system, IAllocator& allocator);

	virtual Entity CreateEntity() = 0;
	virtual void DestroyEntity(Entity entity) = 0;
};


}