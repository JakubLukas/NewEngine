#include "resource_manager.h"


namespace Veng
{


ResourceManager::ResourceManager(IAllocator& allocator)
	: m_allocator(allocator)
	, m_resources(allocator)
{

}

ResourceManager::~ResourceManager()
{

}


Resource* ResourceManager::Load(const Path& path)
{
	return nullptr;
}

void ResourceManager::Unload(Resource& resource)
{

}

void ResourceManager::Reload(Resource& resource)
{

}


}