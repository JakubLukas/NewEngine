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
	virtual ~ResourceManagement() {}


	virtual bool RegisterManager(ResourceType type, ResourceManager* manager) = 0;
	virtual bool UnregisterManager(ResourceType type) = 0;


	virtual resourceHandle LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path) override = 0;
	virtual void UnloadResource(ResourceType resourceType, resourceHandle handle) override = 0;
	virtual void ResourceLoaded(ResourceType resourceType, resourceHandle handle) override = 0;
	virtual const Resource* GetResource(ResourceType resourceType, resourceHandle handle) override = 0;
};


}