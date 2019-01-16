#pragma once

#include "resource.h"


namespace Veng
{


class DependencyManager
{
public:
	virtual resourceHandle LoadResource(ResourceType requestedType, ResourceType resourceType, const Path& path) = 0;
	virtual bool UnloadResource(ResourceType resourceType, resourceHandle handle) = 0;
	virtual void ResourceLoaded(ResourceType resourceType, resourceHandle handle) = 0;
	virtual Resource* GetResource(ResourceType resourceType, resourceHandle handle) = 0;
};


}