#pragma once

#include "core/allocators.h"
#include "resource_manager.h"
#include "core/array.h"


namespace Veng
{


struct DependencyPair
{
	resourceHandle parent;
	resourceHandle child;
};


class DependencyManager
{
public:
	DependencyManager(IAllocator& allocator);
	~DependencyManager();

	void AddResourceManager(ResourceManager* manager)
	{
		m_managers.Push(manager);
	}

private:
	IAllocator& m_allocator;
	Array<DependencyPair> m_dependencies;
	Array<ResourceManager*> m_managers;
};


}