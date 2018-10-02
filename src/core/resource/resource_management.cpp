#include "resource_management.h"

#include "resource_manager.h"


namespace Veng
{


class ResourceManagementImpl : public ResourceManagement
{
public:
	ResourceManagementImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_managers(m_allocator)
		, m_dependencyAsyncOps(allocator)
	{}

	~ResourceManagementImpl()
	{}


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


	resourceHandle LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path) override
	{
		ResourceManager** manager;
		if (m_managers.Find(resourceType, manager))
		{
			DependencyAsyncOp& asyncOp = m_dependencyAsyncOps.Push();
			ResourceManager** reqMng;
			ASSERT(m_managers.Find(requestedType, reqMng));
			asyncOp.parent = *reqMng;
			asyncOp.childType = resourceType;
			asyncOp.childHandle = (*manager)->Load(path);
			return asyncOp.childHandle;
		}
		else
		{
			ASSERT2(false, "Resource manager of given type isn't registered");
			return INVALID_HANDLE;
		}
	}


	void UnloadResource(ResourceType resourceType, resourceHandle handle) override
	{
		ResourceManager** manager;
		if (m_managers.Find(resourceType, manager))
		{
			(*manager)->Unload(handle);
		}
	}


	void ResourceLoaded(ResourceType resourceType, resourceHandle handle) override
	{
		StackAllocator<4096> allocator;
		Array<DependencyAsyncOp> loaded(allocator);//TODO: use temp allocator (stack)

		for (size_t i = 0; i < m_dependencyAsyncOps.GetSize();)
		{
			DependencyAsyncOp& asyncOp = m_dependencyAsyncOps[i];
			if (asyncOp.childType == resourceType && asyncOp.childHandle == handle)
			{
				loaded.Push(asyncOp);
				//tmp.parent->ChildResourceLoaded(handle);
				m_dependencyAsyncOps.Erase((unsigned)i);
			}
			else
			{
				i++;
			}
		}

		for (DependencyAsyncOp& asyncOp : loaded)
		{
			asyncOp.parent->ChildResourceLoaded(handle, resourceType);
		}
	}


	const Resource* GetResource(ResourceType resourceType, resourceHandle handle) override
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
	struct DependencyAsyncOp
	{
		ResourceManager* parent;
		ResourceType childType;
		resourceHandle childHandle;
	};

private:
	IAllocator& m_allocator;
	AssociativeArray<ResourceType, ResourceManager*> m_managers;
	Array<DependencyAsyncOp> m_dependencyAsyncOps;
};


ResourceManagement* ResourceManagement::Create(IAllocator& allocator)
{
	return NEW_OBJECT(allocator, ResourceManagementImpl)(allocator);
}


void ResourceManagement::Destroy(ResourceManagement* system, IAllocator& allocator)
{
	DELETE_OBJECT(allocator, system);
}


}