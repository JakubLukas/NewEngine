#pragma once

#include "core/allocators.h"
#include "core/array.h"
#include "resource.h"


namespace Veng
{


class ResourceManager
{
public:
	ResourceManager(IAllocator& allocator);
	virtual ~ResourceManager();

	Resource* Load(const Path& path);

	void Unload(Resource& resource);

	void Reload(Resource& resource);

private:
	virtual Resource* CreateResource(const Path& path) = 0;
	virtual void DestroyResource(Resource& resource) = 0;

private:
	IAllocator& m_allocator;
	Array<Resource*> m_resources;
};


}