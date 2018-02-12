#pragma once

#include "core/allocators.h"
#include "core/array.h"
#include "core/associative_array.h"

#include "dependency_manager.h"


namespace Veng
{


class ResourceManagement : public DependencyManager
{
public:
	static ResourceManagement* Create(IAllocator& allocator);
	static void Destroy(ResourceManagement* system, IAllocator& allocator);

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


}