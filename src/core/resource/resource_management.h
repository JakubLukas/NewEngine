#pragma once

#include "core/allocator.h"

#include "dependency_manager.h"


namespace Veng
{


class ResourceManagement : public DependencyManager
{
public:
	static ResourceManagement* Create(Allocator& allocator);
	static void Destroy(ResourceManagement* system, Allocator& allocator);

public:
	virtual ~ResourceManagement() {}


	virtual bool RegisterManager(ResourceType type, ResourceManager* manager) = 0;
	virtual bool UnregisterManager(ResourceType type) = 0;
	virtual ResourceManager* GetManager(ResourceType type) const = 0;


	virtual resourceHandle LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path) override = 0;
	virtual bool UnloadResource(ResourceType resourceType, resourceHandle handle) override = 0;
	virtual void ResourceLoaded(ResourceType resourceType, resourceHandle handle) override = 0;
	virtual Resource* GetResource(ResourceType resourceType, resourceHandle handle) override = 0;
};


}