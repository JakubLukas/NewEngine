#include "resource_management.h"

#include "resource_manager.h"


namespace Veng
{


class ResourceManagementImpl : public ResourceManagement
{
public:
	ResourceManagementImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_dependencyAsyncOps(allocator)
	{}

	~ResourceManagementImpl()
	{}


	bool RegisterManager(ResourceType type, ResourceManager* manager)
	{
		if (m_managers[(size_t)type] != nullptr)
			return false;

		m_managers[(size_t)type] = manager;
		return true;
	}


	bool UnregisterManager(ResourceType type)
	{
		if (m_managers[(size_t)type] == nullptr)
			return false;

		m_managers[(size_t)type] = nullptr;
		return true;
	}


	ResourceManager* GetManager(ResourceType type) const override
	{
		ASSERT(m_managers[(size_t)type] != nullptr);
		return m_managers[(size_t)type];
	}


	resourceHandle LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path) override
	{
		ResourceManager* manager = m_managers[(size_t)resourceType];
		if (manager != nullptr)
		{
			DependencyAsyncOp& asyncOp = m_dependencyAsyncOps.PushBack();
			ASSERT(m_managers[(size_t)requestedType] != nullptr);
			asyncOp.parent = m_managers[(size_t)requestedType];
			asyncOp.childType = resourceType;
			asyncOp.childHandle = manager->Load(path);
			return asyncOp.childHandle;
		}
		else
		{
			ASSERT2(false, "Resource manager of given type isn't registered");
			return INVALID_HANDLE;
		}
	}


	bool UnloadResource(ResourceType resourceType, resourceHandle handle) override
	{
		if (m_managers[(size_t)resourceType] == nullptr)
			return false;

		m_managers[(size_t)resourceType]->Unload(handle);
		return true;
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
				loaded.PushBack(asyncOp);
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
		ResourceManager* manager = m_managers[(size_t)resourceType];
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
	IAllocator& m_allocator;
	ResourceManager* m_managers[(size_t)ResourceType::Count] = { nullptr };
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