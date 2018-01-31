#pragma once

#include "core/allocators.h"
#include "resource_manager.h"
#include "core/array.h"


namespace Veng
{


class DependencyManager;

struct DependencyHandler
{
	DependencyHandler(IAllocator& allocator, ResourceManager* parentManager, ResourceManager* childManager)
		: m_parentManager(parentManager), m_childManager(childManager), m_pairs(allocator)
	{}

	void AddDependency(resourceHandle parent, resourceHandle child)
	{
		m_pairs.Push(Pair{ parent, child });

	}

private:
	struct Pair
	{
		resourceHandle parent;
		resourceHandle child;
	};

private:
	ResourceManager* m_parentManager;
	ResourceManager* m_childManager;
	Array<Pair> m_pairs;
};


struct DepResMng
{

	ResourceManager* manager;
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
	Array<DependencyHandler*> m_dependencies;
	Array<ResourceManager*> m_managers;
};


}