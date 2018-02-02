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


	template<class HandleType>
	HandleType LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path)
	{
		ResourceManager** manager;
		if (m_managers.Find(resourceType, manager))
		{
			DependencyTmp& tmp = m_dependenciesTmp.Push();
			ResourceManager** reqMng;
			ASSERT(m_managers.Find(requestedType, reqMng));
			tmp.parent = *reqMng;
			tmp.childType = resourceType;
			tmp.childHandle = (*manager)->LoadInternal(path);
			return static_cast<HandleType>(tmp.childHandle);
		}
		else
		{
			ASSERT2(false, "Resource manager of given type isn't registered");
			return static_cast<HandleType>(INVALID_HANDLE);
		}
	}

	template<class HandleType>
	void UnloadResource(ResourceType resourceType, HandleType handle)
	{
		ResourceManager** manager;
		if (m_managers.Find(resourceType, manager))
		{
			(*manager)->Unload(handle);
		}
	}

	void ResourceLoaded(ResourceType resourceType, resourceHandle handle);

	template<class ResType, class HandleType>
	const ResType* GetResource(ResourceType resourceType, HandleType handle)
	{
		ResourceManager** manager;
		if (m_managers.Find(resourceType, manager))
		{
			return (*manager)->GetResource<ResType>(handle);
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