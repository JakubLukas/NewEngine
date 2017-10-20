#pragma once

#include "core/allocators.h"
#include "core/int.h"


namespace Veng
{


typedef u32 Entity;
//enum Entity : u32 {};


class EntitySystem
{
public:
	static EntitySystem* Create(IAllocator& allocator);
	static void Destroy(EntitySystem* system, IAllocator& allocator);

	virtual Entity CreateEntity() = 0;
	virtual void DestroyEntity(Entity entity) = 0;
};


}