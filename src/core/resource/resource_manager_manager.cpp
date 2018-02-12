#include "resource_manager_manager.h"

#include "resource_manager.h"


namespace Veng
{


class ResourceManagementImpl : public ResourceManagement
{
public:
	//static ResourceManagement* Create(IAllocator& allocator);
	//static void Destroy(ResourceManagement* system, IAllocator& allocator);

public:
	ResourceManagement(IAllocator& allocator);
	~ResourceManagement();


	bool RegisterManager(ResourceType type, ResourceManager* manager);
	bool UnregisterManager(ResourceType type);


	resourceHandle LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path) override;
	void UnloadResource(ResourceType resourceType, resourceHandle handle) override;
	void ResourceLoaded(ResourceType resourceType, resourceHandle handle) override;
	const Resource* GetResource(ResourceType resourceType, resourceHandle handle) override;

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



ResourceManagement::ResourceManagement(IAllocator& allocator)
	: m_allocator(allocator)
	, m_managers(m_allocator)
	, m_dependencyAsyncOps(allocator)
{}


ResourceManagement::~ResourceManagement()
{}


bool ResourceManagement::RegisterManager(ResourceType type, ResourceManager* manager)
{
	ResourceManager** registeredMng;
	if(m_managers.Find(type, registeredMng))
		return false;

	m_managers.Insert(type, manager);
	return true;
}


bool ResourceManagement::UnregisterManager(ResourceType type)
{
	return m_managers.Erase(type);
}


resourceHandle ResourceManagement::LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path)
{
	ResourceManager** manager;
	if(m_managers.Find(resourceType, manager))
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


void ResourceManagement::UnloadResource(ResourceType resourceType, resourceHandle handle)
{
	ResourceManager** manager;
	if(m_managers.Find(resourceType, manager))
	{
		(*manager)->Unload(handle);
	}
}


void ResourceManagement::ResourceLoaded(ResourceType resourceType, resourceHandle handle)
{
	Array<DependencyAsyncOp> loaded(m_allocator);

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
		asyncOp.parent->ChildResourceLoaded(handle);
	}
}


const Resource* ResourceManagement::GetResource(ResourceType resourceType, resourceHandle handle)
{
	ResourceManager** manager;
	if(m_managers.Find(resourceType, manager))
	{
		return (*manager)->GetResource(handle);
	}
	else
	{
		ASSERT2(false, "Resource manager of given type isn't registered");
		return nullptr;
	}
}


ResourceManagement* Create(IAllocator& allocator)
{

}


void Destroy(ResourceManagement* system, IAllocator& allocator)
{

}


}