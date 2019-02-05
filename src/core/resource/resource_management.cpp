#include "resource_management.h"

#include "resource_manager.h"

#include "core/allocators.h"
#include "core/containers/array.h"


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
		ASSERT2(m_managers[(size_t)type] != nullptr, "Manager of requested type is not registered.");
		return m_managers[(size_t)type];
	}


	resourceHandle LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path) override
	{
		ResourceManager* manager = m_managers[(size_t)resourceType];
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
				DependencyAsyncOp& asyncOp = m_dependencyAsyncOps.PushBack();
				asyncOp.parent = parentManager;
				asyncOp.childType = resourceType;
				asyncOp.childHandle = resHandle;
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
		if (m_managers[(size_t)resourceType] == nullptr)
			return false;

		m_managers[(size_t)resourceType]->Unload(handle);
		return true;
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