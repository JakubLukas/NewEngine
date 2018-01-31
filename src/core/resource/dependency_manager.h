#pragma once

#include "core/allocators.h"
#include "resource_manager.h"
#include "core/array.h"


namespace Veng
{


struct DependencyHandler
{
	DependencyHandler(IAllocator& allocator, ResourceManager* parentManager, ResourceManager* childManager)
		: m_parentManager(parentManager), m_childManager(childManager), m_pairs(allocator)
	{}

	resourceHandle LoadDependency(resourceHandle parent, const Path& child)
	{
		//resourceHandle child = m_childManager->L
		//m_pairs.Push(Pair{ parent, child });
	}

	void ChildResourceLoaded(resourceHandle child)
	{
		/*for(size_t i = 0; i < m_pairs.GetSize(); ++i)
		{
			if(m_pairs[i].child == child)
			{
				m_parentManager->ChildResourceLoaded(m_pairs[i].parent, m_pairs[i].child);
				m_pairs.Erase(i);
				i--;
			}
		}*/
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
	DependencyManager(IAllocator& allocator)
		: m_allocator(allocator)
		, m_dependencies(m_allocator)
	{

	}

	~DependencyManager()
	{

	}

	void AddDependency(ResourceManager* parent, ResourceManager* child)
	{
		/*DependencyHandler* handler = NEW_OBJECT(m_allocator, DependencyHandler)(m_allocator, parent, child);
		m_dependencies.Push(handler);
		parent->SetDependencyHandler(handler);
		child->SetDependencyHandlerParent(handler);*/
	}

private:
	IAllocator& m_allocator;
	Array<DependencyHandler*> m_dependencies;
	//Array<ResourceManager*> m_managers;
};


}