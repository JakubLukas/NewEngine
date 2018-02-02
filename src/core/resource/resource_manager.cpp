#include "resource_manager.h"


namespace Veng
{


ResourceManager::ResourceManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: m_allocator(allocator)
	, m_fileSystem(fileSystem)
	, m_loadingTemps(m_allocator)
	, m_resources(m_allocator)
	, m_depManager(depManager)
{
}


ResourceManager::~ResourceManager()
{
	for (auto& tmp : m_loadingTemps)
		ASSERT2(false, "Loading of file in process");
	for (auto& node : m_resources)
		ASSERT2(false, "Resource not released");
}


resourceHandle ResourceManager::LoadInternal(const Path& path)
{
	Resource** item;
	if (!m_resources.Find(path.hash, item))
	{
		Resource* res = CreateResource();
		item = m_resources.Insert(path.hash, res);
		LoadResource(path, *item);
	}

	(*item)->m_refCount++;

	return GetResourceHandle<resourceHandle>(*item);
}


void ResourceManager::UnloadInternal(resourceHandle handle)
{
	Resource* resource = GetResourceInternal(handle);
	u32 hash = resource->m_path.hash;
	Resource** item;
	if (m_resources.Find(hash, item))
	{
		ASSERT(resource == *item);
		if (0 == --resource->m_refCount)
		{
			m_fileSystem.CloseFile((*item)->m_fileHandle);
			DestroyResource(*item);
			m_resources.Erase(hash);
		}
	}
	else
	{
		ASSERT2(false, "Resource with given handle doesn't exist");
	}
}


void ResourceManager::ReloadInternal(resourceHandle handle)
{
	Resource* resource = GetResourceInternal(handle);
	u32 hash = resource->m_path.hash;
	Resource** item;
	if (m_resources.Find(hash, item))
	{
		ASSERT(resource == *item);
		ReloadResource(*item);
	}
	else
	{
		ASSERT2(false, "Resource with given handle doesn't exist");
	}
}


Resource* ResourceManager::GetResourceInternal(resourceHandle handle) const
{
	return reinterpret_cast<Resource*>(handle);
}


void ResourceManager::LoadResource(const Path& path, Resource* resource)
{
	static const FileMode mode{
		FileMode::Access::Read,
		FileMode::ShareMode::ShareRead,
		FileMode::CreationDisposition::OpenExisting,
		FileMode::FlagNone
	};

	ResourceLoadingTemp tmp;
	ASSERT(m_fileSystem.OpenFile(resource->m_fileHandle, path, mode));
	tmp.bufferSize = m_fileSystem.GetSize(resource->m_fileHandle);
	tmp.buffer = m_allocator.Allocate(tmp.bufferSize, ALIGN_OF(char));
	tmp.handle = GetResourceHandle<resourceHandle>(resource);
	Function<void(fileHandle)> f;
	f.Bind<ResourceManager, &ResourceManager::FileSystemCallback>(this);
	ASSERT(m_fileSystem.Read(resource->m_fileHandle, tmp.buffer, tmp.bufferSize, f));

	resource->m_path = path;
	resource->m_state = Resource::State::Loading;

	m_loadingTemps.Insert(resource->m_fileHandle, tmp);
}


void ResourceManager::FileSystemCallback(fileHandle handle)
{
	ResourceLoadingTemp* tmp;
	if (m_loadingTemps.Find(handle, tmp))
	{
		InputBlob blob(tmp->buffer, tmp->bufferSize);

		ResourceLoaded(tmp->handle, blob);

		m_allocator.Deallocate(tmp->buffer);
		ASSERT(m_loadingTemps.Erase(handle));
	}
	else
	{
		ASSERT2(false, "No tmp structure for file handle returned by callback");
	}
}


}