#include "resource_management.h"

#include "resource_manager.h"

#include "core/allocators.h"
#include "core/containers/array.h"
#include "core/containers/hash_map.h"


namespace Veng
{

template<>
struct HashFunc<ResourceType>
{
	static u32 get(const ResourceType& key)
	{
		return key.hash;
	}
};


class ResourceManagementImpl : public ResourceManagement
{
public:
	ResourceManagementImpl(Allocator& allocator)
		: m_allocator(allocator)
		, m_managers(m_allocator)
		, m_dependencyAsyncOps(allocator)
	{}

	~ResourceManagementImpl()
	{}


	bool RegisterManager(ResourceType type, ResourceManager* manager)
	{
		ResourceManager** res;
		if (m_managers.Find(type, res))
			return false;

		m_managers.Insert(type, manager);
		return true;
	}


	bool UnregisterManager(ResourceType type)
	{
		return m_managers.Erase(type);
	}


	ResourceManager* GetManager(ResourceType type) const override
	{
		ResourceManager** res;
		if (!m_managers.Find(type, res))
		{
			ASSERT2(false, "Manager of requested type is not registered.");
			return nullptr;
		}

		return *res;
	}


	resourceHandle LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path) override
	{
		ResourceManager* manager = GetManager(resourceType);
		if (manager != nullptr)
		{
			ResourceManager* parentManager = GetManager(requestedType);
			resourceHandle resHandle = manager->Load(path);
			Resource* resource = manager->GetResource(resHandle);
			if(resource->GetState() == Resource::State::Ready || resource->GetState() == Resource::State::Failure)
			{
				return resHandle;
			}
			else
			{
				DependencyAsyncOp& asyncOp = m_dependencyAsyncOps.PushBack({ parentManager, resourceType, resHandle });
				return asyncOp.childHandle;
			}
		}
		else
		{
			ASSERT2(false, "Resource manager of given type isn't registered");
			return INVALID_RESOURCE_HANDLE;
		}
	}


	bool UnloadResource(ResourceType resourceType, resourceHandle handle) override
	{
		ResourceManager* manager = GetManager(resourceType);
		if (manager != nullptr)
		{
			manager->Unload(handle);
			return true;
		}
		else
		{
			ASSERT2(false, "Resource manager of given type isn't registered");
			return false;
		}
	}


	void ResourceLoaded(ResourceType resourceType, resourceHandle handle) override
	{
		StackAllocator<4096> allocator;
		Array<DependencyAsyncOp> loaded(allocator);

		for (size_t i = 0; i < m_dependencyAsyncOps.GetSize();)
		{
			DependencyAsyncOp& asyncOp = m_dependencyAsyncOps[i];
			if (asyncOp.childType == resourceType && asyncOp.childHandle == handle)
			{
				loaded.PushBack(asyncOp);
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


	Resource* GetResource(ResourceType resourceType, resourceHandle handle) override
	{
		ResourceManager* manager = GetManager(resourceType);
		if (manager != nullptr)
		{
			return manager->GetResource(handle);
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
	Allocator& m_allocator;
	HashMap<ResourceType, ResourceManager*> m_managers;
	Array<DependencyAsyncOp> m_dependencyAsyncOps;
};


ResourceManagement* ResourceManagement::Create(Allocator& allocator)
{
	return NEW_OBJECT(allocator, ResourceManagementImpl)(allocator);
}


void ResourceManagement::Destroy(ResourceManagement* system, Allocator& allocator)
{
	DELETE_OBJECT(allocator, system);
}


}