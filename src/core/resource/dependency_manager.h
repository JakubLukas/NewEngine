#pragma once

#include "core/allocators.h"
#include "resource.h"
#include "resource_manager.h"
#include "core/associative_array.h"


namespace Veng
{


class DependencyManager
{
private:
	struct DependencyTmp
	{
		ResourceManager* parent;
		ResourceType childType;
		resourceHandle childHandle;
	};

public:
	DependencyManager(IAllocator& allocator)
		: m_allocator(allocator)
		, m_managers(m_allocator)
		, m_dependenciesTmp(allocator)
	{
	}

	~DependencyManager()
	{
	}


	bool RegisterManager(ResourceType type, ResourceManager* manager)
	{
		ResourceManager** registeredMng;
		if (m_managers.Find(type, registeredMng))
			return false;
		
		m_managers.Insert(type, manager);
		return true;
	}


	bool UnregisterManager(ResourceType type)
	{
		return m_managers.Erase(type);
	}


	resourceHandle LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path)
	{
		ResourceManager** manager;
		if (m_managers.Find(resourceType, manager))
		{
			DependencyTmp& tmp = m_dependenciesTmp.Push();
			ResourceManager** reqMng;
			ASSERT(m_managers.Find(requestedType, reqMng));
			tmp.parent = *reqMng;
			tmp.childType = resourceType;
			tmp.childHandle = (*manager)->Load(path);
			return tmp.childHandle;
		}
		else
		{
			ASSERT2(false, "Resource manager of given type isn't registered");
			return INVALID_HANDLE;
		}
	}

	void UnloadResource(ResourceType resourceType, resourceHandle handle)
	{
		ResourceManager** manager;
		if (m_managers.Find(resourceType, manager))
		{
			(*manager)->Unload(handle);
		}
	}

	void ResourceLoaded(ResourceType resourceType, resourceHandle handle);

	const Resource* GetResource(ResourceType resourceType, resourceHandle handle)
	{
		ResourceManager** manager;
		if (m_managers.Find(resourceType, manager))
		{
			return (*manager)->GetResource(handle);
		}
		else
		{
			ASSERT2(false, "Resource manager of given type isn't registered");
			return nullptr;
		}
	}

private:
	IAllocator& m_allocator;
	AssociativeArray<ResourceType, ResourceManager*> m_managers;
	Array<DependencyTmp> m_dependenciesTmp;
};


}